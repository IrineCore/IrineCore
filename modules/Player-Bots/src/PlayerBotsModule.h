// =============================================================================
// IrineCore — modules/Player-Bots/src/PlayerBotsModule.h
// Autonomous AI Party Companion Bots Module (excludes Easy/Normal stages).
// =============================================================================
#pragma once

#include "ModuleRegistry.h"

namespace Irine {
namespace Modules {

struct PlayerBotsConfig
{
    bool enabled              = true;
    bool excludeEasyAndNormal = true;
    int  maxBotsPerParty      = 3;
    bool autoSummonInDungeons = true;
    bool autoSummonInNests    = true;
    int  aggroRange           = 1500;
    int  followDistance       = 300;
};

class PlayerBotsModule final : public IModule
{
public:
    const char* GetName() const noexcept override { return "Player-Bots"; }
    bool        IsEnabled() const noexcept override { return m_config.enabled; }

    bool OnLoadConfig(const WCHAR* moduleConfigDirPath) override;

    // Difficulty: 0=Easy, 1=Normal, 2=Hard, 3=Master, 4=Abyss
    bool ShouldAutoSummonBots(int difficulty, bool isNest) const noexcept;
    const PlayerBotsConfig& GetConfig() const noexcept { return m_config; }

private:
    PlayerBotsConfig m_config;
};

} // namespace Modules
} // namespace Irine
