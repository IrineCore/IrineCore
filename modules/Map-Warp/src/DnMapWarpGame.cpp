// =============================================================================
// IrineCore — modules/Map-Warp/src/DnMapWarpGame.cpp
// GameServer server-authoritative smart zone-map warp handler
// (WorldZone / Dungeon / Farm) with navmesh, combat, open-gate, and anti-cheat safety.
// =============================================================================
#include "stdafx.h"
#include "DnMapWarpGame.h"
#include "MapWarpModule.h"

#include "DnPlayerActor.h"
#include "DnNPCActor.h"
#include "DnMonsterActor.h"
#include "DnWorld.h"
#include "DNGameRoom.h"
#include "DNUserSession.h"
#include "EtWorldGrid.h"
#include "EtWorldSector.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <mutex>
#include <unordered_map>

namespace Irine {
namespace Modules {

namespace {

std::mutex s_gameCooldownMutex;
std::unordered_map<UINT, uint64_t> s_gameWarpCooldowns;

uint64_t GetMonotonicMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void NotifyReject(CDNUserSession* pSession, const MapWarpConfig& cfg, const WCHAR* wszMsg)
{
    if (!pSession || !cfg.notifyOnReject || !wszMsg)
        return;
    pSession->SendDebugChat(wszMsg, CHATTYPE_SYSTEM);
}

bool CheckAndConsumeCooldown(UINT nSessionID, uint32_t cooldownMs)
{
    if (cooldownMs == 0)
        return true;

    const uint64_t nowMs = GetMonotonicMs();
    std::lock_guard<std::mutex> lock(s_gameCooldownMutex);

    auto it = s_gameWarpCooldowns.find(nSessionID);
    if (it != s_gameWarpCooldowns.end())
    {
        if (!MapWarpUtil::IsCooldownReady(nowMs, it->second, cooldownMs))
            return false;
        it->second = nowMs;
    }
    else
    {
        if (s_gameWarpCooldowns.size() > 4096)
        {
            for (auto iter = s_gameWarpCooldowns.begin(); iter != s_gameWarpCooldowns.end(); )
            {
                if (nowMs >= iter->second && (nowMs - iter->second) > 300000ULL)
                    iter = s_gameWarpCooldowns.erase(iter);
                else
                    ++iter;
            }
        }
        s_gameWarpCooldowns[nSessionID] = nowMs;
    }
    return true;
}

bool IsPointInsideAnyGateOBB(CDnWorld* pWorld, const EtVector3& vPos, float fExtraMargin)
{
    if (!pWorld)
        return false;

    const DWORD dwGateCount = pWorld->GetGateCount();
    for (DWORD i = 0; i < dwGateCount; ++i)
    {
        GateStruct* pGate = pWorld->GetGateStructFromIndex(i);
        if (!pGate || !pGate->pGateArea)
            continue;

        SOBB* pOBB = pGate->pGateArea->GetOBB();
        if (!pOBB)
            continue;

        EtVector3 vDelta = vPos - pOBB->Center;
        const float proj0 = std::fabs(EtVec3Dot(&vDelta, &pOBB->Axis[0]));
        const float proj2 = std::fabs(EtVec3Dot(&vDelta, &pOBB->Axis[2]));
        if (proj0 <= (pOBB->Extent[0] + fExtraMargin) && proj2 <= (pOBB->Extent[2] + fExtraMargin))
            return true;
    }
    return false;
}

bool IsInDungeonCombat(CMultiRoom* pRoom, CDnPlayerActor* pActor, float fCombatRadius)
{
    if (!pRoom || !pActor)
        return false;

    const EtVector3* pPlayerPos = pActor->GetPosition();
    if (!pPlayerPos)
        return false;

    const int nMyTeam = pActor->GetTeam();
    const float fRadiusSq = fCombatRadius * fCombatRadius;

    DNVector(DnActorHandle) vecActors;
    CDnActor::GetActorList(pRoom, vecActors);

    for (size_t i = 0; i < vecActors.size(); ++i)
    {
        DnActorHandle hAct = vecActors[i];
        if (!hAct || hAct->IsDie() || !hAct->IsShow())
            continue;
        if (!hAct->IsMonsterActor())
            continue;
        if (hAct->GetTeam() == nMyTeam || hAct->GetTeam() == 0)
            continue;

        CDnMonsterActor* pMon = static_cast<CDnMonsterActor*>(hAct.GetPointer());
        if (!pMon)
            continue;

        // Actively aggroed hostile monster anywhere in stage
        DnActorHandle hAggro = pMon->GetAggroTarget();
        if (hAggro && !hAggro->IsDie() && hAggro->GetTeam() == nMyTeam)
            return true;

        // Hostile monster standing right near the player
        const EtVector3* pMonPos = pMon->GetPosition();
        if (pMonPos)
        {
            const float dx = pMonPos->x - pPlayerPos->x;
            const float dz = pMonPos->z - pPlayerPos->z;
            if ((dx * dx + dz * dz) <= fRadiusSq)
                return true;
        }
    }

    return false;
}

bool EvaluateGameCandidate(CDnWorld* pWorld, float candX, float candZ, float refY,
                           float gateExtraMargin, float maxHeightDelta,
                           EtVector3& outPos)
{
    if (!pWorld)
        return false;
    if (!std::isfinite(candX) || !std::isfinite(candZ) || !std::isfinite(refY))
        return false;

    EtVector3 vCand(candX, refY, candZ);

    // Must lie on walkable navigation mesh (if map has a navmesh)
    NavigationMesh* pNavMesh = pWorld->GetNavMesh(vCand);
    if (pNavMesh)
    {
        if (!pWorld->IsOnNavigationMesh(vCand))
            return false;
    }
    else
    {
        // Fallback for maps without navmesh: reject blocked terrain attributes
        if (pWorld->GetAttribute(vCand) != 0)
            return false;
    }

    vCand.y = pWorld->GetHeightWithProp(vCand);
    if (!MapWarpUtil::IsHeightDeltaSafe(refY, vCand.y, maxHeightDelta))
        return false;

    // Never place the player inside any portal trigger box (avoids instant zone-change loops)
    if (IsPointInsideAnyGateOBB(pWorld, vCand, gateExtraMargin))
        return false;

    outPos = vCand;
    return true;
}

CEtWorldEventArea* FindStartPositionArea(CDnWorld* pWorld, int nGateIndex)
{
    if (!pWorld || !pWorld->GetGrid() || nGateIndex <= 0)
        return NULL;

    char szStartName[32] = { 0 };
    sprintf_s(szStartName, "StartPosition %d", nGateIndex);

    const DWORD dwCount = pWorld->GetGrid()->GetActiveSectorCount();
    for (DWORD i = 0; i < dwCount; ++i)
    {
        CEtWorldSector* pSector = pWorld->GetGrid()->GetActiveSector(i);
        if (!pSector)
            continue;
        CEtWorldEventControl* pControl = pSector->GetControlFromUniqueID(ETE_EventArea);
        if (!pControl)
            continue;
        CEtWorldEventArea* pArea = pControl->GetAreaFromName(szStartName);
        if (pArea)
            return pArea;
    }
    return NULL;
}

bool ResolveGameNpcTarget(CMultiRoom* pRoom, CDnWorld* pWorld, const MapWarpConfig& cfg,
                          int nNpcID, EtVector3& outDest, EtVector2& outLook)
{
    if (!pRoom || !pWorld || nNpcID <= 0)
        return false;

    DNVector(DnActorHandle) vecActors;
    CDnActor::GetActorList(pRoom, vecActors);

    CDnNPCActor* pTargetNpc = NULL;
    for (size_t i = 0; i < vecActors.size(); ++i)
    {
        DnActorHandle hAct = vecActors[i];
        if (!hAct || !hAct->IsNpcActor() || !hAct->IsShow())
            continue;
        CDnNPCActor* pNpc = static_cast<CDnNPCActor*>(hAct.GetPointer());
        if (pNpc && pNpc->GetNpcData() && pNpc->GetNpcData()->nNpcID == nNpcID)
        {
            pTargetNpc = pNpc;
            break;
        }
    }

    if (!pTargetNpc || !pTargetNpc->GetPosition())
        return false;

    const EtVector3 vNpcPos = *pTargetNpc->GetPosition();
    float fwdX = 0.f;
    float fwdZ = 1.f;
    if (pTargetNpc->GetMatEx())
    {
        MapWarpUtil::NormalizeDirection2D(
            pTargetNpc->GetMatEx()->m_vZAxis.x,
            pTargetNpc->GetMatEx()->m_vZAxis.z,
            fwdX, fwdZ);
    }

    const float distances[3] = {
        cfg.npcFrontDistance,
        (std::max)(70.f, cfg.npcFrontDistance * 0.75f),
        cfg.npcFrontDistance * 1.45f
    };

    for (int d = 0; d < 3; ++d)
    {
        for (size_t i = 0; i < MapWarpUtil::kCandidateAngleOffsetCount; ++i)
        {
            float cx = 0.f, cz = 0.f;
            MapWarpUtil::ComputeCandidatePoint(
                vNpcPos.x, vNpcPos.z, fwdX, fwdZ, distances[d],
                MapWarpUtil::kCandidateAngleOffsetsDeg[i], cx, cz);

            if (EvaluateGameCandidate(pWorld, cx, cz, vNpcPos.y, 20.f, cfg.maxHeightDelta, outDest))
            {
                float lx = 0.f, lz = 1.f;
                MapWarpUtil::ComputeLookToward(outDest.x, outDest.z, vNpcPos.x, vNpcPos.z, lx, lz);
                outLook = EtVector2(lx, lz);
                return true;
            }
        }
    }

    return false;
}

bool ResolveGamePortalTarget(CDnWorld* pWorld, const MapWarpConfig& cfg,
                             int nGateIndex, bool& outGateClosed,
                             EtVector3& outDest, EtVector2& outLook)
{
    outGateClosed = false;
    if (!pWorld || nGateIndex <= 0 || nGateIndex > DNWORLD_GATE_COUNT)
        return false;

    GateStruct* pGate = pWorld->GetGateStruct(static_cast<char>(nGateIndex));
    if (!pGate || !pGate->pGateArea)
        return false;

    if (cfg.requireOpenGate && pGate->PermitFlag != EWorldEnum::PermitEnter)
    {
        outGateClosed = true;
        return false;
    }

    SOBB* pGateOBB = pGate->pGateArea->GetOBB();
    if (!pGateOBB)
        return false;

    const EtVector3 vGateCenter = pGateOBB->Center;
    EtVector3 vGateFloorProbe(vGateCenter.x, vGateCenter.y, vGateCenter.z);
    const float gateRefY = pWorld->GetHeightWithProp(vGateFloorProbe);

    // Priority 1: Map's StartPosition <nGateIndex> area (authoritative landing zone in front of the portal)
    CEtWorldEventArea* pStartArea = FindStartPositionArea(pWorld, nGateIndex);
    if (pStartArea && pStartArea->GetOBB())
    {
        const EtVector3 vSpCenter = pStartArea->GetOBB()->Center;
        const float spRefY = pWorld->GetHeight(vSpCenter.x, vSpCenter.z) + pStartArea->GetMin()->y;

        if (EvaluateGameCandidate(pWorld, vSpCenter.x, vSpCenter.z, spRefY, 20.f, cfg.maxHeightDelta, outDest))
        {
            float lx = 0.f, lz = 1.f;
            MapWarpUtil::ComputeLookToward(outDest.x, outDest.z, vGateCenter.x, vGateCenter.z, lx, lz);
            outLook = EtVector2(lx, lz);
            return true;
        }

        // Nudge away from gate center if StartPosition slightly touches the safety margin
        float awayX = 0.f, awayZ = 1.f;
        if (MapWarpUtil::NormalizeDirection2D(vSpCenter.x - vGateCenter.x, vSpCenter.z - vGateCenter.z, awayX, awayZ))
        {
            const float nudges[3] = { 60.f, 120.f, 200.f };
            for (int n = 0; n < 3; ++n)
            {
                const float nx = vSpCenter.x + awayX * nudges[n];
                const float nz = vSpCenter.z + awayZ * nudges[n];
                if (EvaluateGameCandidate(pWorld, nx, nz, spRefY, 20.f, cfg.maxHeightDelta, outDest))
                {
                    float lx = 0.f, lz = 1.f;
                    MapWarpUtil::ComputeLookToward(outDest.x, outDest.z, vGateCenter.x, vGateCenter.z, lx, lz);
                    outLook = EtVector2(lx, lz);
                    return true;
                }
            }
        }
    }

    // Priority 2: Sweep safe navmesh stand points outside the gate OBB
    float fwdX = 0.f, fwdZ = 1.f;
    MapWarpUtil::DirectionFromYawDegrees(pGate->pGateArea->GetRotate(), fwdX, fwdZ);
    const float baseRadius = (std::max)(pGateOBB->Extent[0], pGateOBB->Extent[2]) + cfg.portalFrontDistance;

    const float distances[3] = {
        baseRadius,
        baseRadius + 100.f,
        baseRadius + 220.f
    };

    for (int d = 0; d < 3; ++d)
    {
        for (size_t i = 0; i < MapWarpUtil::kCandidateAngleOffsetCount; ++i)
        {
            float cx = 0.f, cz = 0.f;
            MapWarpUtil::ComputeCandidatePoint(
                vGateCenter.x, vGateCenter.z, fwdX, fwdZ, distances[d],
                MapWarpUtil::kCandidateAngleOffsetsDeg[i], cx, cz);

            if (EvaluateGameCandidate(pWorld, cx, cz, gateRefY, cfg.gateSafetyMargin, cfg.maxHeightDelta, outDest))
            {
                float lx = 0.f, lz = 1.f;
                MapWarpUtil::ComputeLookToward(outDest.x, outDest.z, vGateCenter.x, vGateCenter.z, lx, lz);
                outLook = EtVector2(lx, lz);
                return true;
            }
        }
    }

    return false;
}

} // anonymous namespace

bool HandleGameMapWarp(CDnPlayerActor* pActor, uint8_t cTargetType, int nTargetID)
{
    if (!pActor || pActor->IsAIBotActor())
        return false;

    const MapWarpConfig& cfg = MapWarpModule::GetActiveConfig();
    if (!cfg.enabled)
        return false;

    if (cTargetType == MAP_WARP_TARGET_NPC && !cfg.allowNpcWarp)
        return false;
    if (cTargetType == MAP_WARP_TARGET_PORTAL && !cfg.allowPortalWarp)
        return false;
    if (cTargetType != MAP_WARP_TARGET_NPC && cTargetType != MAP_WARP_TARGET_PORTAL)
        return false;

    CDNUserSession* pSession = pActor->GetUserSession();
    if (!pSession || pSession->GetState() != SESSION_STATE_GAME_PLAY)
        return false;

    CMultiRoom* pRoom = pActor->GetRoom();
    if (!pRoom)
        return false;

    CDNGameRoom* pGameRoom = static_cast<CDNGameRoom*>(pRoom);
    if (pGameRoom->GetRoomState() != _GAME_STATE_PLAY)
        return false;

    CDnWorld* pWorld = CDnWorld::GetInstancePtr(pRoom);
    if (!pWorld)
        return false;

    // Map / mode permissions
    const bool bIsPvP = pGameRoom->bIsPvPRoom() || (pGameRoom->GetGameTaskType() == GameTaskType::PvP);
    const bool bIsDarkLair = (pGameRoom->GetGameTaskType() == GameTaskType::DarkLair) ||
                             (pWorld->GetMapSubType() == EWorldEnum::MapSubTypeDarkLair);
    const bool bIsFarm = pGameRoom->bIsFarmRoom() ||
                         (pGameRoom->GetGameTaskType() == GameTaskType::Farm) ||
                         (pWorld->GetMapSubType() == EWorldEnum::MapSubTypeFarm);

    if (bIsPvP && !cfg.allowInPvP)
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Warping is disabled in PvP.");
        return false;
    }
    if (bIsDarkLair && !cfg.allowInDarkLair)
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Warping is disabled in Dark Lair.");
        return false;
    }
    if (bIsFarm)
    {
        if (!cfg.allowInFarm)
        {
            NotifyReject(pSession, cfg, L"[MapWarp] Warping is disabled in Farm zones.");
            return false;
        }
    }
    else if (!bIsPvP && !bIsDarkLair && !cfg.allowInDungeon)
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Warping is disabled in dungeons.");
        return false;
    }

    // Actor safety state checks (prevent breaking death, hit-stun, air/down, skills, cannons, grabs, keep-ops)
    if (pActor->IsDie() || pActor->IsGhost() || pActor->IsSpectatorMode())
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Cannot warp while incapacitated.");
        return false;
    }
    if (pActor->IsHit() || pActor->IsAir() || pActor->IsDown() || pActor->IsProcessSkill())
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Cannot warp while performing an action.");
        return false;
    }
    if (pActor->IsCannonMode() || pSession->GetKeepOperationID() > 0)
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Cannot warp while interacting with an object.");
        return false;
    }
#ifdef PRE_ADD_MONSTER_CATCH
    if (pActor->IsCatchedByMonster())
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Cannot warp while restrained.");
        return false;
    }
#endif

    // Dungeon combat guard (WorldZones & Farms have no combat; dungeons enforce out-of-combat safety)
    if (!bIsFarm && cfg.requireOutOfCombatInDungeon &&
        pWorld->GetMapType() == EWorldEnum::MapTypeDungeon)
    {
        if (IsInDungeonCombat(pRoom, pActor, static_cast<float>(cfg.combatCheckRadius)))
        {
            NotifyReject(pSession, cfg, L"[MapWarp] Cannot warp while in combat.");
            return false;
        }
    }

    // Cooldown check
    if (!CheckAndConsumeCooldown(pSession->GetSessionID(), static_cast<uint32_t>(cfg.cooldownMs)))
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Please wait a moment before warping again.");
        return false;
    }

    EtVector3 vDest(0.f, 0.f, 0.f);
    EtVector2 vLook(0.f, 1.f);
    bool bResolved = false;
    bool bGateClosed = false;

    if (cTargetType == MAP_WARP_TARGET_NPC)
        bResolved = ResolveGameNpcTarget(pRoom, pWorld, cfg, nTargetID, vDest, vLook);
    else if (cTargetType == MAP_WARP_TARGET_PORTAL)
        bResolved = ResolveGamePortalTarget(pWorld, cfg, nTargetID, bGateClosed, vDest, vLook);

    if (!bResolved)
    {
        if (bGateClosed)
            NotifyReject(pSession, cfg, L"[MapWarp] That portal is currently locked.");
        else
            NotifyReject(pSession, cfg, L"[MapWarp] Could not find a safe landing position near that target.");
        return false;
    }

    // Reset anti-cheat movement checkers cleanly before authoritative teleport
    pActor->ResetWarpAntiCheatCheckers();
    pActor->SetPosition(vDest);
    pActor->SetStaticPosition(vDest);
    pActor->SetPrevPosition(vDest);
    pActor->CmdStop("Stand");
    pActor->CmdWarp(vDest, vLook, NULL, true);

    return true;
}

} // namespace Modules
} // namespace Irine
