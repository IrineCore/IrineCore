// =============================================================================
// IrineCore — modules/Map-Warp/src/MapWarpModule.h
// Smart Zone-Map NPC & Portal Warping Module configuration & pure math helpers.
// =============================================================================
#pragma once

#include "ModuleRegistry.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace Irine {
namespace Modules {

enum MapWarpTargetType : uint8_t
{
    MAP_WARP_TARGET_NONE   = 0,
    MAP_WARP_TARGET_NPC    = 1,
    MAP_WARP_TARGET_PORTAL = 2,
};

struct MapWarpConfig
{
    // [MapWarp]
    bool enabled         = true;
    bool allowNpcWarp    = true;
    bool allowPortalWarp = true;
    int  cooldownMs      = 1500;
    bool notifyOnReject  = true;

    // [Rules]
    bool allowInVillage              = true;
    bool allowInDungeon              = true;
    bool allowInFarm                 = true;
    bool allowInPvP                  = false;
    bool allowInDarkLair             = false;
    bool requireOutOfCombatInDungeon = true;
    int  combatCheckRadius           = 1000;
    bool requireOpenGate             = true;

    // [Positioning]
    float npcFrontDistance    = 110.f;
    float portalFrontDistance = 140.f;
    float gateSafetyMargin    = 90.f;
    float maxHeightDelta      = 150.f;
};

// Pure geometry & validation helpers shared by VillageServer, GameServer, and unit tests
namespace MapWarpUtil
{
    // Sweep offsets around the target's front vector (degrees): front first, then symmetric fans, then rear
    static const float kCandidateAngleOffsetsDeg[] = {
        0.f, 25.f, -25.f, 50.f, -50.f, 80.f, -80.f, 115.f, -115.f, 150.f, -150.f, 180.f
    };
    static const size_t kCandidateAngleOffsetCount = sizeof(kCandidateAngleOffsetsDeg) / sizeof(kCandidateAngleOffsetsDeg[0]);

    bool  NormalizeDirection2D(float dx, float dz, float& outX, float& outZ) noexcept;
    void  DirectionFromYawDegrees(float yawDegrees, float& outX, float& outZ) noexcept;
    void  ComputeCandidatePoint(float originX, float originZ,
                                float forwardX, float forwardZ,
                                float distance, float angleOffsetDeg,
                                float& outX, float& outZ) noexcept;
    void  ComputeLookToward(float fromX, float fromZ,
                            float targetX, float targetZ,
                            float& outLookX, float& outLookZ) noexcept;
    float ComputeYawDegreesFromLook(float lookX, float lookZ) noexcept;
    bool  IsPointInsideBox2D(float x, float z,
                             float minX, float minZ,
                             float maxX, float maxZ,
                             float margin = 0.f) noexcept;
    bool  IsHeightDeltaSafe(float referenceY, float candidateY, float maxDelta) noexcept;
    float SelectBestFloorHeight(float referenceY, float terrainY, float collisionY,
                                bool hasCollisionHeight, float maxDelta, bool& outValid) noexcept;
    bool  IsCooldownReady(uint64_t nowMs, uint64_t lastWarpMs, uint32_t cooldownMs) noexcept;
}

class MapWarpModule final : public IModule
{
public:
    MapWarpModule();
    ~MapWarpModule() override = default;

    const char* GetName() const noexcept override { return "Map-Warp"; }
    bool        IsEnabled() const noexcept override { return m_config.enabled; }

    bool OnLoadConfig(const WCHAR* moduleConfigDirPath) override;

    const MapWarpConfig& GetConfig() const noexcept { return m_config; }
    static const MapWarpConfig& GetActiveConfig() noexcept;

private:
    MapWarpConfig m_config;
};

void Addmod_Map_WarpScripts();

} // namespace Modules
} // namespace Irine
