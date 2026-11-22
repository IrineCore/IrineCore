// =============================================================================
// IrineCore — modules/Map-Warp/src/DnMapWarpVillage.cpp
// VillageServer server-authoritative smart zone-map warp handler.
// =============================================================================
#include "stdafx.h"
#include "DnMapWarpVillage.h"
#include "MapWarpModule.h"

#include "DNUserSession.h"
#include "DNField.h"
#include "DNFieldData.h"
#include "DNNpcObject.h"
#include "DnWorldGateStruct.h"
#include "Log.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <mutex>
#include <unordered_map>

namespace Irine {
namespace Modules {

namespace {

std::mutex s_villageCooldownMutex;
std::unordered_map<UINT, uint64_t> s_villageWarpCooldowns;

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
    std::lock_guard<std::mutex> lock(s_villageCooldownMutex);

    auto it = s_villageWarpCooldowns.find(nSessionID);
    if (it != s_villageWarpCooldowns.end())
    {
        if (!MapWarpUtil::IsCooldownReady(nowMs, it->second, cooldownMs))
            return false;
        it->second = nowMs;
    }
    else
    {
        if (s_villageWarpCooldowns.size() > 4096)
        {
            // Prune stale entries older than 5 minutes to keep memory bounded
            for (auto iter = s_villageWarpCooldowns.begin(); iter != s_villageWarpCooldowns.end(); )
            {
                if (nowMs >= iter->second && (nowMs - iter->second) > 300000ULL)
                    iter = s_villageWarpCooldowns.erase(iter);
                else
                    ++iter;
            }
        }
        s_villageWarpCooldowns[nSessionID] = nowMs;
    }
    return true;
}

bool EvaluateVillageCandidate(CDNField* pField, CDNFieldData* pFieldData,
                              float candX, float candZ, float refY,
                              float gateExtraMargin, float maxHeightDelta,
                              TPosition& outPos)
{
    if (!pField || !pFieldData)
        return false;
    if (!std::isfinite(candX) || !std::isfinite(candZ) || !std::isfinite(refY))
        return false;

    // Never place the player inside any portal trigger zone (prevents accidental map loads/loops)
    if (pFieldData->IsInsideAnyGateTrigger(candX, candZ, gateExtraMargin))
        return false;

    const float terrainY = pFieldData->GetTerrainHeight(candX, candZ);
    const float collY = pFieldData->HasCollisionHeight() ? pFieldData->GetCollisionHeight(candX, candZ) : terrainY;

    bool bHeightValid = false;
    const float bestY = MapWarpUtil::SelectBestFloorHeight(
        refY, terrainY, collY, pFieldData->HasCollisionHeight(), maxHeightDelta, bHeightValid);
    if (!bHeightValid)
        return false;

    TPosition testPos = { 0, 0, 0 };
    testPos.nX = static_cast<int>(candX * 1000.f);
    testPos.nY = static_cast<int>(bestY * 1000.f);
    testPos.nZ = static_cast<int>(candZ * 1000.f);

    // Must lie within an active field block of the current map
    if (pField->GetFieldBlock(&testPos) == NULL)
        return false;

    outPos = testPos;
    return true;
}

bool ResolveVillageNpcTarget(CDNField* pField, CDNFieldData* pFieldData,
                             const MapWarpConfig& cfg, int nNpcID,
                             TPosition& outDest, float& outLookX, float& outLookZ)
{
    if (nNpcID <= 0)
        return false;

    CDNNpcObject* pNpc = pField->FindNpcObjectByNpcID(nNpcID);
    if (!pNpc)
        return false;

    const TPosition npcPos = pNpc->GetCurrentPos();
    const float npcX = static_cast<float>(npcPos.nX) / 1000.f;
    const float npcY = static_cast<float>(npcPos.nY) / 1000.f;
    const float npcZ = static_cast<float>(npcPos.nZ) / 1000.f;

    float fwdX = 0.f;
    float fwdZ = 1.f;
    MapWarpUtil::DirectionFromYawDegrees(pNpc->GetRotate(), fwdX, fwdZ);

    const float distances[3] = {
        cfg.npcFrontDistance,
        (std::max)(70.f, cfg.npcFrontDistance * 0.75f),
        cfg.npcFrontDistance * 1.45f
    };

    for (int d = 0; d < 3; ++d)
    {
        for (size_t i = 0; i < MapWarpUtil::kCandidateAngleOffsetCount; ++i)
        {
            float cx = 0.f;
            float cz = 0.f;
            MapWarpUtil::ComputeCandidatePoint(
                npcX, npcZ, fwdX, fwdZ, distances[d],
                MapWarpUtil::kCandidateAngleOffsetsDeg[i], cx, cz);

            if (EvaluateVillageCandidate(pField, pFieldData, cx, cz, npcY, 20.f, cfg.maxHeightDelta, outDest))
            {
                MapWarpUtil::ComputeLookToward(cx, cz, npcX, npcZ, outLookX, outLookZ);
                return true;
            }
        }
    }

    return false;
}

bool ResolveVillagePortalTarget(CDNField* pField, CDNFieldData* pFieldData,
                                const MapWarpConfig& cfg, int nGateIndex,
                                TPosition& outDest, float& outLookX, float& outLookZ)
{
    if (nGateIndex <= 0 || nGateIndex > DNWORLD_GATE_COUNT)
        return false;

    float gateMinX = 0.f, gateMinZ = 0.f, gateMaxX = 0.f, gateMaxZ = 0.f, gateAngle = 0.f;
    if (!pFieldData->GetGateAreaBounds(nGateIndex, gateMinX, gateMinZ, gateMaxX, gateMaxZ, gateAngle))
        return false;

    const float gateCX = (gateMinX + gateMaxX) * 0.5f;
    const float gateCZ = (gateMinZ + gateMaxZ) * 0.5f;
    const float gateHalfX = (std::max)(50.f, (gateMaxX - gateMinX) * 0.5f);
    const float gateHalfZ = (std::max)(50.f, (gateMaxZ - gateMinZ) * 0.5f);
    const float gateRefY = pFieldData->GetTerrainHeight(gateCX, gateCZ);

    // Priority 1: Use the map's hand-authored StartPosition <nGateIndex> center (guaranteed safe landing zone)
    float spX = 0.f, spY = 0.f, spZ = 0.f, spAngle = 0.f;
    if (pFieldData->GetStartPositionCenter(nGateIndex, spX, spY, spZ, spAngle))
    {
        if (EvaluateVillageCandidate(pField, pFieldData, spX, spZ, spY, 15.f, cfg.maxHeightDelta, outDest))
        {
            MapWarpUtil::ComputeLookToward(spX, spZ, gateCX, gateCZ, outLookX, outLookZ);
            return true;
        }

        // If StartPosition touches a padded gate edge, nudge gently away from the gate center
        float awayX = 0.f, awayZ = 1.f;
        if (MapWarpUtil::NormalizeDirection2D(spX - gateCX, spZ - gateCZ, awayX, awayZ))
        {
            const float nudges[3] = { 60.f, 120.f, 200.f };
            for (int n = 0; n < 3; ++n)
            {
                const float nx = spX + awayX * nudges[n];
                const float nz = spZ + awayZ * nudges[n];
                if (EvaluateVillageCandidate(pField, pFieldData, nx, nz, spY, 15.f, cfg.maxHeightDelta, outDest))
                {
                    MapWarpUtil::ComputeLookToward(nx, nz, gateCX, gateCZ, outLookX, outLookZ);
                    return true;
                }
            }
        }
    }

    // Priority 2: Sweep safe stand points just outside the gate trigger box
    float fwdX = 0.f, fwdZ = 1.f;
    MapWarpUtil::DirectionFromYawDegrees(gateAngle, fwdX, fwdZ);
    const float baseRadius = (std::max)(gateHalfX, gateHalfZ) + cfg.portalFrontDistance;

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
                gateCX, gateCZ, fwdX, fwdZ, distances[d],
                MapWarpUtil::kCandidateAngleOffsetsDeg[i], cx, cz);

            if (EvaluateVillageCandidate(pField, pFieldData, cx, cz, gateRefY, cfg.gateSafetyMargin, cfg.maxHeightDelta, outDest))
            {
                MapWarpUtil::ComputeLookToward(cx, cz, gateCX, gateCZ, outLookX, outLookZ);
                return true;
            }
        }
    }

    return false;
}

} // anonymous namespace

bool HandleVillageMapWarp(CDNUserSession* pSession, uint8_t cTargetType, int nTargetID)
{
    if (!pSession)
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

    CDNField* pField = pSession->GetField();
    if (!pField)
        return false;

    CDNFieldData* pFieldData = pField->GetFieldData();
    if (!pFieldData)
        return false;

    // Map-type permissions
    if (pField->bIsPvPLobby())
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Warping is not available in the Colosseum lobby.");
        return false;
    }
    if (pField->bIsPvPVillage() && !cfg.allowInPvP)
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Warping is disabled in PvP areas.");
        return false;
    }
    if (pField->bIsDarkLairVillage() && !cfg.allowInDarkLair)
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Warping is disabled in this area.");
        return false;
    }
    if (pField->bIsFarmVillage())
    {
        if (!cfg.allowInFarm)
        {
            NotifyReject(pSession, cfg, L"[MapWarp] Warping is disabled in Farm zones.");
            return false;
        }
    }
    else if (!pField->bIsPvPVillage() && !pField->bIsDarkLairVillage() && !cfg.allowInVillage)
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Warping is disabled in towns.");
        return false;
    }

    // Player busy / state safety checks (trade, chatroom, map transition, PvP/ladder)
    if (pSession->IsBusyForMapWarp())
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Cannot warp while busy.");
        return false;
    }

    // Cooldown check (checked before resolving geometry to protect server CPU from spam)
    if (!CheckAndConsumeCooldown(pSession->GetSessionID(), static_cast<uint32_t>(cfg.cooldownMs)))
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Please wait a moment before warping again.");
        return false;
    }

    TPosition destPos = { 0, 0, 0 };
    float lookX = 0.f;
    float lookZ = 1.f;
    bool bResolved = false;

    if (cTargetType == MAP_WARP_TARGET_NPC)
        bResolved = ResolveVillageNpcTarget(pField, pFieldData, cfg, nTargetID, destPos, lookX, lookZ);
    else if (cTargetType == MAP_WARP_TARGET_PORTAL)
        bResolved = ResolveVillagePortalTarget(pField, pFieldData, cfg, nTargetID, destPos, lookX, lookZ);

    if (!bResolved)
    {
        NotifyReject(pSession, cfg, L"[MapWarp] Could not find a safe landing position near that target.");
        return false;
    }

    const float fRotateDeg = MapWarpUtil::ComputeYawDegreesFromLook(lookX, lookZ);
    return pSession->SmartWarpInField(destPos, lookX, lookZ, fRotateDeg);
}

} // namespace Modules
} // namespace Irine
