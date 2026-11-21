# Changelog

All notable changes to **IrineCore — OpenSource Dragon Nest Server Emulator** are documented in this file.

## [1.0.0] - 2026-11-20

### Architecture & Build System
- Established unified cross-platform CMake build hierarchy (`deps/`, `common/`, `modules/`, `src/{cmake,server,client}/`, `tests/`).
- Isolated OS-specific implementations behind `common/platform/{interfaces,windows,linux}` (`irine_platform`).
- Replaced proprietary DirectX 9 SDK math headers with portable C++ math library (`deps/etmath/EtMathPortable.h`).
- Standardized `cmake --install` / `make install` distribution staging into `env/dist/` (`config/`, `module/`, `systems/`, `<server-binaries>`, `<dlls>`).
- Added multi-stage `Dockerfile`, `docker-compose.yml`, and `apps/{compiler,installer,docker}/` automation scripts.

### Modular Extensions (`modules/`)
- Standardized module layout: `modules/<ModuleName>/{src,modules,resource,data,CMakeLists.txt}`.
- Built `Player-Bots` (`mod_player_bots`) as a standalone static library with per-module configuration templates installed to `env/dist/module/`.

### Compatibility & Testing
- Preserved 100% wire compatibility with the unmodified Windows `DragonNest.exe` Cap 60 client and external SQL Server databases (`DNMembership`, `DNWorld`, `DNServerLog`).
- Added automated `CTest` verification suite (`IrineCoreTestSuite`) covering platform/math, module framework, wire packet ABI, and server configuration integrity.
