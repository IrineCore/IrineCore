// =============================================================================
// IrineCore — modules/Map-Warp/src/MapWarpModule.cpp
// Smart Zone-Map NPC & Portal Warping Module implementation & math helpers.
// =============================================================================
#include "MapWarpModule.h"
#include <cmath>
#include <cstdio>

namespace Irine {
namespace Modules {

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kDegToRad = kPi / 180.0f;
constexpr float kRadToDeg = 180.0f / kPi;

int ClampInt(int v, int lo, int hi) noexcept
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float ClampFloat(float v, float lo, float hi) noexcept
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

int ReadIniInt(const WCHAR* section, const WCHAR* key, int defaultVal, const std::wstring& path)
{
    return static_cast<int>(::GetPrivateProfileIntW(section, key, defaultVal, path.c_str()));
}

} // anonymous namespace

namespace MapWarpUtil {

bool NormalizeDirection2D(float dx, float dz, float& outX, float& outZ) noexcept
{
    const float lenSq = dx * dx + dz * dz;
    if (!std::isfinite(lenSq) || lenSq < 1e-6f)
    {
        outX = 0.f;
        outZ = 1.f;
        return false;
    }
    const float invLen = 1.f / std::sqrt(lenSq);
    outX = dx * invLen;
    outZ = dz * invLen;
    return true;
}

void DirectionFromYawDegrees(float yawDegrees, float& outX, float& outZ) noexcept
{
    if (!std::isfinite(yawDegrees))
        yawDegrees = 0.f;
    const float rad = yawDegrees * kDegToRad;
    outX = std::sin(rad);
    outZ = std::cos(rad);
    NormalizeDirection2D(outX, outZ, outX, outZ);
}

void ComputeCandidatePoint(float originX, float originZ,
                           float forwardX, float forwardZ,
                           float distance, float angleOffsetDeg,
                           float& outX, float& outZ) noexcept
{
    float nx = 0.f;
    float nz = 1.f;
    NormalizeDirection2D(forwardX, forwardZ, nx, nz);

    const float rad = angleOffsetDeg * kDegToRad;
    const float c = std::cos(rad);
    const float s = std::sin(rad);

    // Rotate 2D XZ vector around +Y axis
    const float rx = nx * c + nz * s;
    const float rz = -nx * s + nz * c;

    outX = originX + rx * distance;
    outZ = originZ + rz * distance;
}

void ComputeLookToward(float fromX, float fromZ,
                       float targetX, float targetZ,
                       float& outLookX, float& outLookZ) noexcept
{
    NormalizeDirection2D(targetX - fromX, targetZ - fromZ, outLookX, outLookZ);
}

float ComputeYawDegreesFromLook(float lookX, float lookZ) noexcept
{
    float nx = 0.f;
    float nz = 1.f;
    NormalizeDirection2D(lookX, lookZ, nx, nz);
    float deg = std::atan2(nx, nz) * kRadToDeg;
    if (deg < 0.f)
        deg += 360.f;
    return deg;
}

bool IsPointInsideBox2D(float x, float z,
                        float minX, float minZ,
                        float maxX, float maxZ,
                        float margin) noexcept
{
    if (!std::isfinite(x) || !std::isfinite(z))
        return true; // Treat non-finite coordinates as unsafe / inside hazard
    const float loX = (minX < maxX ? minX : maxX) - margin;
    const float hiX = (minX < maxX ? maxX : minX) + margin;
    const float loZ = (minZ < maxZ ? minZ : maxZ) - margin;
    const float hiZ = (minZ < maxZ ? maxZ : minZ) + margin;
    return (x >= loX && x <= hiX && z >= loZ && z <= hiZ);
}

bool IsHeightDeltaSafe(float referenceY, float candidateY, float maxDelta) noexcept
{
    if (!std::isfinite(referenceY) || !std::isfinite(candidateY) || !std::isfinite(maxDelta))
        return false;
    return std::fabs(candidateY - referenceY) <= maxDelta;
}

float SelectBestFloorHeight(float referenceY, float terrainY, float collisionY,
                            bool hasCollisionHeight, float maxDelta, bool& outValid) noexcept
{
    const bool terrainOk = IsHeightDeltaSafe(referenceY, terrainY, maxDelta);
    const bool collOk = hasCollisionHeight && IsHeightDeltaSafe(referenceY, collisionY, maxDelta);

    if (terrainOk && collOk)
    {
        outValid = true;
        // Prefer the valid surface closest to target reference elevation (handles elevated bridges/platforms
        // while avoiding snapping onto overhead roofs or falling under prop decks).
        const float dTerrain = std::fabs(terrainY - referenceY);
        const float dColl = std::fabs(collisionY - referenceY);
        return (dColl <= dTerrain) ? collisionY : terrainY;
    }
    if (collOk)
    {
        outValid = true;
        return collisionY;
    }
    if (terrainOk)
    {
        outValid = true;
        return terrainY;
    }

    outValid = false;
    return referenceY;
}

bool IsCooldownReady(uint64_t nowMs, uint64_t lastWarpMs, uint32_t cooldownMs) noexcept
{
    if (cooldownMs == 0 || lastWarpMs == 0)
        return true;
    if (nowMs < lastWarpMs)
        return true; // Defensive against clock wrap/reset
    return (nowMs - lastWarpMs) >= static_cast<uint64_t>(cooldownMs);
}

} // namespace MapWarpUtil

MapWarpModule::MapWarpModule() = default;

bool MapWarpModule::OnLoadConfig(const WCHAR* moduleConfigDirPath)
{
    const std::wstring configPath = ResolveModuleConfigPath(moduleConfigDirPath, this);
    if (configPath.empty())
        return true;

    // [MapWarp]
    m_config.enabled         = (ReadIniInt(L"MapWarp", L"Enabled", 1, configPath) != 0);
    m_config.allowNpcWarp    = (ReadIniInt(L"MapWarp", L"AllowNpcWarp", 1, configPath) != 0);
    m_config.allowPortalWarp = (ReadIniInt(L"MapWarp", L"AllowPortalWarp", 1, configPath) != 0);
    m_config.cooldownMs      = ClampInt(ReadIniInt(L"MapWarp", L"CooldownMs", 1500, configPath), 0, 60000);
    m_config.notifyOnReject  = (ReadIniInt(L"MapWarp", L"NotifyOnReject", 1, configPath) != 0);

    // [Rules]
    m_config.allowInVillage              = (ReadIniInt(L"Rules", L"AllowInVillage", 1, configPath) != 0);
    m_config.allowInDungeon              = (ReadIniInt(L"Rules", L"AllowInDungeon", 1, configPath) != 0);
    m_config.allowInFarm                 = (ReadIniInt(L"Rules", L"AllowInFarm", 1, configPath) != 0);
    m_config.allowInPvP                  = (ReadIniInt(L"Rules", L"AllowInPvP", 0, configPath) != 0);
    m_config.allowInDarkLair             = (ReadIniInt(L"Rules", L"AllowInDarkLair", 0, configPath) != 0);
    m_config.requireOutOfCombatInDungeon = (ReadIniInt(L"Rules", L"RequireOutOfCombatInDungeon", 1, configPath) != 0);
    m_config.combatCheckRadius           = ClampInt(ReadIniInt(L"Rules", L"CombatCheckRadius", 1000, configPath), 200, 4000);
    m_config.requireOpenGate             = (ReadIniInt(L"Rules", L"RequireOpenGate", 1, configPath) != 0);

    // [Positioning]
    m_config.npcFrontDistance    = ClampFloat(static_cast<float>(ReadIniInt(L"Positioning", L"NpcFrontDistance", 110, configPath)), 50.f, 400.f);
    m_config.portalFrontDistance = ClampFloat(static_cast<float>(ReadIniInt(L"Positioning", L"PortalFrontDistance", 140, configPath)), 60.f, 500.f);
    m_config.gateSafetyMargin    = ClampFloat(static_cast<float>(ReadIniInt(L"Positioning", L"GateSafetyMargin", 90, configPath)), 30.f, 350.f);
    m_config.maxHeightDelta      = ClampFloat(static_cast<float>(ReadIniInt(L"Positioning", L"MaxHeightDelta", 150, configPath)), 30.f, 600.f);

    return true;
}

const MapWarpConfig& MapWarpModule::GetActiveConfig() noexcept
{
    static const MapWarpConfig s_DefaultConfig;
    IModule* pMod = ModuleRegistry::Instance().GetModule("Map-Warp");
    if (pMod)
    {
        auto* pWarp = dynamic_cast<MapWarpModule*>(pMod);
        if (pWarp)
            return pWarp->GetConfig();
    }
    return s_DefaultConfig;
}

void Addmod_Map_WarpScripts()
{
    ModuleRegistry::Instance().RegisterModule(std::unique_ptr<IModule>(new MapWarpModule()));
}

} // namespace Modules
} // namespace Irine
