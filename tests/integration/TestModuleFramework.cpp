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

    return failures;
}
