// =============================================================================
// IrineCore — tests/integration/TestModuleFramework.cpp
// Integration tests for the IrineCore ModuleRegistry & ModuleLoader.
// Works cleanly whether all modules are enabled, selectively disabled, or all disabled.
// =============================================================================
#include "ModuleRegistry.h"
#include "ModuleLoader.h"
#ifdef IRINE_MOD_PLAYER_BOTS
#include "PlayerBotsModule.h"
#endif
#include <cstdio>

int RunIntegrationTests(const WCHAR* moduleConfigDirPath)
{
    int failures = 0;
    auto check = [&](bool cond, const char* msg) {
        if (!cond) { std::fprintf(stderr, "  [FAIL][Integration] %s\n", msg); ++failures; }
        else       { std::printf("  [PASS][Integration] %s\n", msg); }
    };

    check(Irine::Modules::LoadAllModules(moduleConfigDirPath),
          "Irine::Modules::LoadAllModules loaded all enabled module .ini files");

    auto& reg = Irine::Modules::ModuleRegistry::Instance();

    // Auto-verify every registered module resolves its config dynamically without hardcoding
    for (const auto& mod : reg.GetModules())
    {
        const std::wstring cfgPath = Irine::Modules::ResolveModuleConfigPath(moduleConfigDirPath, mod.get());
        check(!cfgPath.empty(), "Module config path auto-resolved from module registry");
    }

#ifdef IRINE_MOD_PLAYER_BOTS
    {
        using namespace Irine::Modules;
        auto* bots = dynamic_cast<PlayerBotsModule*>(reg.GetModule("Player-Bots"));
        check(bots != nullptr && bots->IsEnabled(), "Player-Bots module registered and enabled");
        if (bots)
        {
            check(!bots->ShouldAutoSummonBots(0, false) && !bots->ShouldAutoSummonBots(1, false),
                  "Player-Bots excluded on Easy (0) and Normal (1) difficulty stages");
            check(bots->ShouldAutoSummonBots(2, false) && bots->ShouldAutoSummonBots(3, false) && bots->ShouldAutoSummonBots(4, false),
                  "Player-Bots active on Hard (2), Master (3), and Abyss (4) difficulty stages");

            // Verify expanded INI sections loaded cleanly via GetActiveConfig()
            const PlayerBotsConfig& cfg = PlayerBotsModule::GetActiveConfig();
            check(cfg.syncLevelToPlayer && cfg.unlockJobByLevel && cfg.resyncOnStageTransition,
                  "Player-Bots [Scaling] config loaded");
            check(cfg.gearSyncMode == GEAR_SYNC_MATCH_PLAYER && cfg.matchPlayerEnchant && cfg.matchPlayerGlyphs && cfg.matchPlayerTalismans,
                  "Player-Bots [Gear] config loaded");
            check(cfg.syncSkillsToLevel && cfg.respectSkillPrerequisites && cfg.healerManaReservePercent == 25,
                  "Player-Bots [Skills] config loaded");
            check(cfg.applyInstanceStrategies && cfg.allowBotChatCommands && cfg.assistHumanLeader,
                  "Player-Bots [Strategies] & [CombatAI] config loaded");

            // Unit-test job tier unlocking by level (Base Lv 1-14, 1st Job Lv 15-44, 2nd Job Lv 45-60)
            check(BotStrategyUtil::ResolveUnlockedJobTier(1, true) == 0 &&
                  BotStrategyUtil::ResolveUnlockedJobTier(14, true) == 0 &&
                  BotStrategyUtil::ResolveUnlockedJobTier(15, true) == 1 &&
                  BotStrategyUtil::ResolveUnlockedJobTier(44, true) == 1 &&
                  BotStrategyUtil::ResolveUnlockedJobTier(45, true) == 2 &&
                  BotStrategyUtil::ResolveUnlockedJobTier(60, true) == 2 &&
                  BotStrategyUtil::ResolveUnlockedJobTier(5, false) == 2,
                  "BotStrategyUtil::ResolveUnlockedJobTier matches Dragon Nest class tiers");

            // Unit-test level syncing with clamping & offset
            PlayerBotsConfig lvlCfg = cfg;
            lvlCfg.levelOffset = -2;
            lvlCfg.minBotLevel = 5;
            lvlCfg.maxBotLevel = 60;
            check(BotStrategyUtil::ComputeSyncedBotLevel(5, lvlCfg) == 5 &&
                  BotStrategyUtil::ComputeSyncedBotLevel(20, lvlCfg) == 18 &&
                  BotStrategyUtil::ComputeSyncedBotLevel(60, lvlCfg) == 58,
                  "BotStrategyUtil::ComputeSyncedBotLevel applies offset and clamps to [min,max]");

            // Unit-test gear rank & enchant scaling (match player vs baseline vs fixed endgame)
            PlayerBotsConfig gearCfg = cfg;
            gearCfg.gearSyncMode = GEAR_SYNC_MATCH_PLAYER;
            check(BotStrategyUtil::ComputeSyncedGearRank(7, 4, 1.2f, gearCfg) == 1 &&
                  BotStrategyUtil::ComputeSyncedGearRank(60, 7, 3.8f, gearCfg) == 4 &&
                  BotStrategyUtil::ComputeSyncedEnchantLevel(7, 1, 4, 2.2f, gearCfg) == 2 &&
                  BotStrategyUtil::ComputeSyncedEnchantLevel(60, 4, 7, 10.1f, gearCfg) == 10,
                  "BotStrategyUtil gear rank & enchant sync tracks human equipment accurately");

            gearCfg.gearSyncMode = GEAR_SYNC_LEVEL_ONLY;
            check(BotStrategyUtil::ComputeSyncedGearRank(7, 4, 4.0f, gearCfg) == 1 &&
                  BotStrategyUtil::ComputeSyncedGearRank(55, 4, 1.0f, gearCfg) == 3,
                  "BotStrategyUtil GEAR_SYNC_LEVEL_ONLY uses level-appropriate baseline");

            // Unit-test composable combat & non-combat strategy parser & mutual exclusions
            std::string unknown;
            uint32_t coMask = BotStrategyUtil::ApplyCombatStrategyModifiers(
                CS_DPS | CS_AOE | CS_BURST, "+focus_boss,-burst,+defensive,bogus_flag", &unknown);
            check((coMask & CS_FOCUS_BOSS) != 0 && (coMask & CS_DEFENSIVE) != 0 &&
                  (coMask & CS_BURST) == 0 && (coMask & CS_AOE) != 0 && unknown == "bogus_flag",
                  "BotStrategyUtil::ApplyCombatStrategyModifiers handles +, -, and unknown tokens");

            coMask = BotStrategyUtil::ApplyCombatStrategyModifiers(coMask, "+clear_adds");
            check((coMask & CS_CLEAR_ADDS) != 0 && (coMask & CS_FOCUS_BOSS) == 0,
                  "BotStrategyUtil::ApplyCombatStrategyModifiers enforces focus_boss/clear_adds mutual exclusion");

            uint32_t ncMask = BotStrategyUtil::ApplyNonCombatStrategyModifiers(NCS_FOLLOW | NCS_BUFF, "+stay,+passive");
            check((ncMask & NCS_STAY) != 0 && (ncMask & NCS_FOLLOW) == 0 && (ncMask & NCS_PASSIVE) != 0,
                  "BotStrategyUtil::ApplyNonCombatStrategyModifiers enforces stay/follow mutual exclusion");

            check(!BotStrategyUtil::FormatCombatStrategies(coMask).empty() &&
                  !BotStrategyUtil::FormatNonCombatStrategies(ncMask).empty(),
                  "BotStrategyUtil strategy formatters produce readable labels");
        }
    }
#else
    check(reg.GetModule("Player-Bots") == nullptr, "Player-Bots cleanly excluded when disabled");
#endif

    return failures;
}
