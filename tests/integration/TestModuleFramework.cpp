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
#ifdef IRINE_MOD_HERO_ALLY
#include "HeroAllyModule.h"
#endif
#ifdef IRINE_MOD_FPS_PING
#include "FpsPingModule.h"
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

#ifdef IRINE_MOD_PLAYER_BOTS
    {
        auto* bots = dynamic_cast<Irine::Modules::PlayerBotsModule*>(reg.GetModule("Player-Bots"));
        check(bots != nullptr && bots->IsEnabled(), "Player-Bots module registered and enabled");
        if (bots)
        {
            check(!bots->ShouldAutoSummonBots(0, false) && !bots->ShouldAutoSummonBots(1, false),
                  "Player-Bots excluded on Easy (0) and Normal (1) difficulty stages");
            check(bots->ShouldAutoSummonBots(2, false) && bots->ShouldAutoSummonBots(3, false) && bots->ShouldAutoSummonBots(4, false),
                  "Player-Bots active on Hard (2), Master (3), and Abyss (4) difficulty stages");
        }
    }
#else
    check(reg.GetModule("Player-Bots") == nullptr, "Player-Bots cleanly excluded when disabled");
#endif

#ifdef IRINE_MOD_HERO_ALLY
    {
        auto* hero = dynamic_cast<Irine::Modules::HeroAllyModule*>(reg.GetModule("Hero-Ally"));
        check(hero != nullptr && hero->IsEnabled(), "Hero-Ally module registered and enabled");
        if (hero)
        {
            check(hero->ShouldSpawnHeroAlly(0) && hero->ShouldSpawnHeroAlly(1),
                  "Hero-Ally active on Easy (0) and Normal (1) difficulty stages");
            check(!hero->ShouldSpawnHeroAlly(2) && !hero->ShouldSpawnHeroAlly(3) && !hero->ShouldSpawnHeroAlly(4),
                  "Hero-Ally excluded on Hard (2), Master (3), and Abyss (4) difficulty stages");
            check(hero->SelectHeroAllyMonsterId(0) == 236005 &&
                  hero->SelectHeroAllyMonsterId(1) == 236003 &&
                  hero->SelectHeroAllyMonsterId(2) == 236011,
                  "Hero-Ally rotates Geraint (236005), Argenta (236003), and Velskud (236011)");
        }
    }
#else
    check(reg.GetModule("Hero-Ally") == nullptr, "Hero-Ally cleanly excluded when disabled");
#endif

#ifdef IRINE_MOD_FPS_PING
    {
        auto* ping = dynamic_cast<Irine::Modules::FpsPingModule*>(reg.GetModule("FPS-PING"));
        check(ping != nullptr && ping->IsEnabled(), "FPS-PING module registered and enabled");
        if (ping)
        {
            check(ping->GetConfig().pingUpdateIntervalMs == 1000,
                  "FPS-PING loaded PingUpdateIntervalMs=1000 from modules/FPS-PING/modules/FPS-PING.ini");
        }
    }
#else
    check(reg.GetModule("FPS-PING") == nullptr, "FPS-PING cleanly excluded when disabled");
#endif

    return failures;
}
