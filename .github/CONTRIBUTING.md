# Contributing to IrineCore

Thank you for your interest in contributing to **IrineCore — OpenSource Dragon Nest Server Emulator**!

## Core Engineering Rules

Before opening a pull request, please make sure your changes respect IrineCore's non-negotiable guarantees:

1. **Database Preservation**: Do not alter table names, column names, indexes, or stored procedure signatures in `DNMembership`, `DNWorld`, or `DNServerLog`.
2. **Client Wire Compatibility**: Preserve `#pragma pack(1)` layout on wire packets, `sizeof(WCHAR) == 2` (`-fshort-wchar` on Linux), TEA packet encryption, and unmodified `DragonNest.exe` protocol compatibility.
3. **Cross-Platform Parity**: Keep gameplay and server logic OS-agnostic. Place OS-specific code under `common/platform/windows/` or `common/platform/linux/` behind `common/platform/interfaces/`.
4. **Module Isolation**: Implement custom gameplay features as standalone static library modules under `modules/<ModuleName>/` rather than cluttering core `src/server/` or `game/` subsystems.
5. **Unified Logging Format**: Use the standard IrineCore logging format:
   `<timestamp> [<severity>] [<file>:<line>] [<function>] : <message>`

---

## Creating a New Module

Every module under `modules/` follows this standardized layout:

```text
modules/<ModuleName>/
├── CMakeLists.txt           # Defines static library target + irine_install_module_configs()
├── src/                     # Module C++ headers and sources
├── modules/
│   └── <ModuleName>.ini     # Default module configuration template (installed to env/dist/module/)
├── resource/                # Optional custom assets (.ui, .dnt, etc.)
└── data/                    # Optional custom MSSQL tables/procedures
```

Register your module in `modules/ModuleLoader.cpp` and add `add_subdirectory(<ModuleName>)` to `modules/CMakeLists.txt`.

---

## Building & Testing Locally

### Windows (MSVC x64)
```powershell
.\apps\compiler\compile.ps1
```

### Linux (Ubuntu 22.04–26.04 LTS / Debian 12+)
```bash
./apps/compiler/compile.sh
```

Both scripts configure CMake, compile all dependencies, core libraries, modules, and all 7 servers, run the automated `ctest` suite (`IrineCoreTestSuite`), and install the distribution into `env/dist/`.

---

## Pull Request Process

1. Fork the repository and create a feature branch from `main` (e.g. `feat/my-module` or `fix/login-packet`).
2. Keep commits focused, descriptive, and cleanly scoped.
3. Verify that all targets build without errors and `ctest --test-dir build --output-on-failure` passes.
4. Fill out the Pull Request template completely and clearly describe how you tested your change.
