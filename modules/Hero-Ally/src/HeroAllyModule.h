// =============================================================================
// IrineCore — modules/Hero-Ally/src/HeroAllyModule.h
// Hero Ally Accompaniment Module (Geraint, Argenta, Velskud on Easy/Normal).
// =============================================================================
#pragma once

#include "ModuleRegistry.h"
#include <cstdint>

namespace Irine {
namespace Modules {

struct HeroAllyConfig
{
    bool enabled              = true;
    int  triggerChancePercent = 100;
    bool spawnAtStageStart    = true;
    bool followAcrossAreas    = true;
    bool despawnOnStageClear  = true;
    int  geraintMonsterId     = 236005;
    int  argentaMonsterId     = 236003;
    int  velskudMonsterId     = 236011;
};

class HeroAllyModule final : public IModule
{
public:
    const char* GetName() const noexcept override { return "Hero-Ally"; }
    const char* GetConfigFileName() const noexcept override { return "Hero-Ally.ini"; }
    bool        IsEnabled() const noexcept override { return m_config.enabled; }

    bool OnLoadConfig(const WCHAR* moduleConfigDirPath) override;

    // Easy (0) and Normal (1) stages only
    bool ShouldSpawnHeroAlly(int difficulty) const noexcept;
    int  SelectHeroAllyMonsterId(uint32_t seed) const noexcept;
    const HeroAllyConfig& GetConfig() const noexcept { return m_config; }

private:
    HeroAllyConfig m_config;
};

} // namespace Modules
} // namespace Irine
