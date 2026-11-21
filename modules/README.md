# IrineCore — Modular Extension System (`modules/`)

IrineCore implements a **dynamic modular extension framework** (`src/cmake/IrineModules.cmake` + auto-generated `ModuleLoader.cpp`). Modules placed inside `modules/` are automatically discovered at CMake configure time and can be **enabled or completely excluded from compilation and linking without deleting their folders or editing core source code**.

## Disabling Modules While Keeping Folders in `modules/`

You can exclude any or all modules using any of the four supported mechanisms:

1. **Disable ALL modules globally (pure vanilla core)**:
   ```powershell
   cmake -S . -B build -A x64 -DIRINE_MODULES=disabled
   ```
2. **Disable specific modules via semicolon-separated list** (`DISABLED_IRINE_MODULES`):
   ```powershell
   cmake -S . -B build -A x64 -DDISABLED_IRINE_MODULES="Player-Bots;examples"
   ```
3. **Disable a single module via its auto-generated CMake option** (`IRINE_MODULE_<UPPER_SANITIZED_NAME>`):
   ```powershell
   cmake -S . -B build -A x64 -DIRINE_MODULE_PLAYER_BOTS=OFF
   ```
4. **Drop a `.disabled` marker file** inside `modules/<ModuleName>/.disabled`, or move the folder into `modules/disabled/<ModuleName>/`.

Disabled modules are **never compiled, never linked, never registered in the generated `ModuleLoader.cpp`, and their configs are not installed or listed in `module/modules.manifest`**.

### Automatic Module Config Discovery
Module configuration files (`modules/*.ini`, `modules/*.conf`, `modules/*.conf.dist`, `conf/*.ini`, `conf/*.conf`, `conf/*.conf.dist`) are discovered automatically at CMake configure time:
- `cmake --install` installs all enabled module configs into `env/dist/module/` and generates `env/dist/module/modules.manifest`.
- Deployment and CI validation scripts (`deploy/windows/configure.ps1`, `deploy/linux/configure.sh`, `.github/workflows/windows_build.yml`) read `modules.manifest` automatically with no hardcoded filenames.
- At runtime, modules call `ResolveModuleConfigPath(moduleConfigDirPath, this)` without overriding `GetConfigFileName()`, automatically probing `<ModName>.ini`, `<ModName>.conf`, and `<ModName>.conf.dist` across flat and source-tree layouts.

## Standardized Module Directory Layout

```text
modules/
├── CMakeLists.txt               # Dynamic module discovery + irine_modules library
├── ModuleLoader.h               # Public loader API: RegisterBuiltinModules(), LoadAllModules()
├── ModuleLoader.cpp.in          # CMake template -> build/generated/ModuleLoader.cpp
└── <ModuleName>/
    ├── CMakeLists.txt           # Optional: calls irine_register_module(...) (auto-built if omitted!)
    ├── src/                     # Module C++ sources implementing Addmod_<SanitizedName>Scripts()
    ├── modules/ (or conf/)      # Template .ini / .conf.dist (installed to env/dist/module/ when enabled)
    ├── resource/                # Optional custom client/server assets (.ui, .dnt, etc.)
    └── data/sql/                # Optional custom SQL updates (installed to env/dist/module/sql/)
```

### Drop-In Community Module Entry Point
Every module exports a registration function whose name replaces non-alphanumeric characters (such as `-`) with `_`:

```cpp
// modules/mod-my-feature/src/MyFeature.cpp
#include "ModuleRegistry.h"

namespace Irine {
namespace Modules {

void Addmod_mod_my_featureScripts()
{
    ModuleRegistry::Instance().RegisterModule(std::unique_ptr<IModule>(new MyFeatureModule()));
}

} // namespace Modules
} // namespace Irine
```

Drop `modules/mod-my-feature/` into `modules/` and re-run CMake — no core file edits required.

## Built-in Modules
- **`modules/Player-Bots`** (`IRINE_MODULE_PLAYER_BOTS`, `IRINE_MOD_PLAYER_BOTS`):
  Autonomous AI party companion bots (`DnAIBotBuildData.{h,cpp}`, `DnAIBotController.{h,cpp}`, `PlayerBotsModule.{h,cpp}`). Summons and controls AI party members on Hard, Master, and Abyss dungeons and Nests while excluding Easy and Normal stages.
- **`modules/examples`** (`IRINE_MODULE_EXAMPLES`, `IRINE_MOD_EXAMPLES`):
  Reference template demonstrating `Addmod_<Name>Scripts()` registration and standardized module layout.
