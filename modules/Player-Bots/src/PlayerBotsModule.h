// =============================================================================
// IrineCore — modules/Player-Bots/src/PlayerBotsModule.h
// Autonomous AI Party Companion Bots Module with modular strategy engine,
// player-synced level/gear/skill scaling, and full INI configuration.
// =============================================================================
#pragma once

#include "ModuleRegistry.h"
#include <cstdint>
#include <string>

namespace Irine {
namespace Modules {

// Modular combat strategies (composable via config & live party chat commands)
enum CombatStrategyFlags : uint32_t
{
    CS_NONE         = 0,
    CS_TANK         = 1u << 0,
    CS_HEALER       = 1u << 1,
    CS_DPS          = 1u << 2,
    CS_ICESTACK     = 1u << 3,
    CS_SUPPORT      = 1u << 4,
    CS_AOE          = 1u << 5,
    CS_BURST        = 1u << 6,
    CS_THREAT       = 1u << 7,
    CS_AVOID_AOE    = 1u << 8,
    CS_DEFENSIVE    = 1u << 9,
    CS_ASSIST       = 1u << 10,
    CS_FOCUS_BOSS   = 1u << 11,
    CS_CLEAR_ADDS   = 1u << 12,
    CS_SAVE_MANA    = 1u << 13,
    CS_BREAK_PROPS  = 1u << 14,
    CS_DODGE_CANCEL = 1u << 15,
};

// Modular non-combat strategies
enum NonCombatStrategyFlags : uint32_t
{
    NCS_NONE        = 0,
    NCS_FOLLOW      = 1u << 0,
    NCS_STAY        = 1u << 1,
    NCS_BUFF        = 1u << 2,
    NCS_GATHER_GATE = 1u << 3,
    NCS_PASSIVE     = 1u << 4,
};

enum GearSyncModeEnum : int
{
    GEAR_SYNC_FIXED_ENDGAME = 0,
    GEAR_SYNC_MATCH_PLAYER  = 1,
    GEAR_SYNC_LEVEL_ONLY    = 2,
};

struct PlayerBotsConfig
{
    // [PlayerBots]
    bool enabled                 = true;
    bool excludeEasyAndNormal    = true;
    int  maxBotsPerParty         = 7;
    bool autoSummonInDungeons    = true;
    bool autoSummonInNests       = true;
    bool useRecommendedPartySize = true;

    // [Scaling]
    bool syncLevelToPlayer       = true;
    int  levelOffset             = 0;
    int  minBotLevel             = 1;
    int  maxBotLevel             = 60;
    bool unlockJobByLevel        = true;
    bool resyncOnStageTransition = true;

    // [Gear]
    int  gearSyncMode            = GEAR_SYNC_MATCH_PLAYER;
    int  gearQualityOffset       = 0;
    int  minGearRank             = 0; // ITEMRANK_D (Normal)
    int  maxGearRank             = 5; // ITEMRANK_SS (Heroic)
    bool matchPlayerEnchant      = true;
    int  enchantOffset           = 0;
    int  maxEnchantLevel         = 12;
    bool matchPlayerPotentials   = true;
    bool matchPlayerCashCostumes = true;
    bool matchPlayerGlyphs       = true;
    bool matchPlayerTalismans    = true;

    // [Skills]
    bool syncSkillsToLevel        = true;
    bool respectSkillPrerequisites = true;
    int  skillPowerPercent        = 0;  // 0 = match player's average skill rank ratio
    int  healerManaReservePercent = 25;
    int  dpsManaConservePercent   = 15;

    // [Strategies]
    std::string defaultCombatStrategies    = "+aoe,+burst,+threat,+avoid_aoe,+assist,+save_mana,+break_props,+dodge_cancel";
    std::string defaultNonCombatStrategies = "+follow,+buff,+gather_gate";
    bool        applyInstanceStrategies    = true;

    // [CombatAI]
    int  aggroRange               = 1500;
    int  followDistance           = 300;
    int  globalSkillDelayMs       = 260;
    int  reactionJitterMs         = 55;
    int  healThresholdDungeon     = 75;
    int  healThresholdNest        = 82;
    int  criticalHealThreshold    = 42;
    int  selfDefensiveHpThreshold = 35;
    int  overkillSkipHpPercent    = 18;
    int  allySpreadRadius         = 115;
    bool assistHumanLeader        = true;
    bool allowBotChatCommands     = true;
    bool enableTacticalCallouts   = true;
};

// Pure helpers shared between GameServer and test suite
namespace BotStrategyUtil
{
    // Job tier: 0 = Base Class (Lv 1-14), 1 = 1st Job (Lv 15-44), 2 = 2nd Job (Lv 45-60)
    int ResolveUnlockedJobTier(int level, bool unlockByLevel) noexcept;
    int ComputeSyncedBotLevel(int humanLevel, const PlayerBotsConfig& cfg) noexcept;
    int ComputeBaselineGearRankForLevel(int botLevel) noexcept;
    int ComputeSyncedGearRank(int botLevel, int humanEquippedCount, float humanAvgRank, const PlayerBotsConfig& cfg) noexcept;
    int ComputeSyncedEnchantLevel(int botLevel, int targetRank, int humanEquippedCount, float humanAvgEnchant, const PlayerBotsConfig& cfg) noexcept;

    // Strategy parser & formatter (+name, -name, name, reset)
    uint32_t ApplyCombatStrategyModifiers(uint32_t currentMask, const std::string& expr, std::string* pOutUnknown = nullptr);
    uint32_t ApplyNonCombatStrategyModifiers(uint32_t currentMask, const std::string& expr, std::string* pOutUnknown = nullptr);
    std::string FormatCombatStrategies(uint32_t mask);
    std::string FormatNonCombatStrategies(uint32_t mask);
}

class PlayerBotsModule final : public IModule
{
public:
    const char* GetName() const noexcept override { return "Player-Bots"; }
    bool        IsEnabled() const noexcept override { return m_config.enabled; }

    bool OnLoadConfig(const WCHAR* moduleConfigDirPath) override;

    // Difficulty: 0=Easy, 1=Normal, 2=Hard, 3=Master, 4=Abyss
    bool ShouldAutoSummonBots(int difficulty, bool isNest) const noexcept;
    const PlayerBotsConfig& GetConfig() const noexcept { return m_config; }

    // Safe accessor for GameServer components (falls back to default config if module not registered)
    static const PlayerBotsConfig& GetActiveConfig() noexcept;

private:
    PlayerBotsConfig m_config;
};

} // namespace Modules
} // namespace Irine
