# AGENTS.md

Working rules and engineering guidelines for contributors and automated agents working in IrineCore.

## Project Scope

IrineCore is an open-source Dragon Nest (Cap 60) server emulator and Windows client/launcher codebase:

- **Servers**: 7 cross-platform x64 executables (`DNLogServer64`, `DNDBServer64`, `DNCashServer64`, `DNLoginServer64`, `DNMasterServer64`, `DNVillageServer64`, `DNGameServer64`) targeting Windows and Linux.
- **Client & Launcher**: Windows-only (`DragonNest.exe`, `DnLauncher.exe`), built natively for x64 by default (Win32 also supported).
- **Modules**: Opt-in gameplay and utility extensions under `modules/`, auto-discovered by CMake and safe to disable without breaking core builds.

## Hard Invariants

1. **Wire Protocol & ABI Compatibility**
   - Keep `#pragma pack(push, 1)` / `#pragma pack(pop)` intact on all client-server and inter-server packet structs.
   - `WCHAR` must remain 2-byte UTF-16LE (`static_assert(sizeof(WCHAR) == 2)`). Linux builds enforce this with `-fshort-wchar`; never pass `WCHAR*` directly to glibc `wchar_t` functions (use the UTF-16 helpers in `common/platform/linux/`).
   - Do not break TEA packet encryption, rolling sequence framing, or RUDP framing.

2. **Database Schema Compatibility**
   - `data/sql/base/` is the single source of truth for all 5 SQL Server databases (`dngsm`, `dnmembership`, `dnserverlog`, `dnstaging`, `dnworld`).
   - Preserve stored procedure names, parameter order/types, and existing table/column names expected by the C++ server code.
   - Put additive schema patches in `data/sql/updates/` using idempotent T-SQL (`IF OBJECT_ID(...) IS NULL`, `CREATE OR ALTER PROCEDURE`).
   - Keep MSSQL Docker initialization building directly from `data/sql/base/`; do not commit duplicated pre-bundled SQL copies under `deploy/mssql/`.

3. **Module Isolation**
   - Implement optional gameplay features as self-contained modules under `modules/<ModuleName>/` exporting `Addmod_<SanitizedName>Scripts()`.
   - Any core hook into a module must be guarded by `#ifdef IRINE_MOD_<UPPER_NAME>` with clean no-op fallbacks so the server and test suite always build and pass when `-DIRINE_MODULES=disabled` or individual modules are turned off.
   - Do not add module include directories or module libraries to core CMake files directly; use `irine_register_module(...)` and `irine_link_gameserver_modules(...)`.

4. **Cross-Platform Server Code**
   - Isolate OS-specific APIs behind `common/platform/{interfaces,windows,linux}`.
   - Match `#include` filename casing to the exact on-disk filename (Linux filesystems are case-sensitive).
   - Do not introduce DirectX headers or Win32-only APIs into server targets; use `deps/etmath/EtMathPortable.h` for server 3D math.
   - Keep server code region-agnostic; do not reintroduce country/publisher `#ifdef` branches or `Nation=` INI keys.

5. **Client & Launcher Rules**
   - Both `DragonNest.exe` and `DnLauncher.exe` must compile cleanly as native x64 (and remain Win32-clean).
   - Use pointer-width Win32 APIs (`SetWindowLongPtr`/`GetWindowLongPtr`, `GWLP_*`, `UINT_PTR`, `SHANDLE_PTR`, `INVALID_HANDLE_VALUE`); never cast pointers or handles to 32-bit `DWORD`/`int`/`LONG`.
   - Do not add 32-bit-only precompiled `.lib` dependencies; use native Windows APIs (e.g., GDI+ in `DnLauncher`) or full-source static libraries.
   - Keep a single canonical `RT_MANIFEST` at Resource ID `1` requesting `requireAdministrator` (`uiAccess="false"`) via `/MANIFESTUAC` in CMake. Never embed a second manifest in `.rc` files.

6. **Logging & Source Hygiene**
   - Write all new comments, log messages, and docs in clear English ASCII.
   - Keep server log entries consistent with the unified log format in `common/utilities/LogFormat.h` and `common/logging/Log.h`.
   - Never hardcode machine-specific absolute paths in code, CMake, scripts, or documentation.

## Build, Test & Verification Workflow

- **Windows Servers (x64)**:
  ```powershell
  cmake -S . -B build -A x64
  cmake --build build --config Release --parallel
  ctest --test-dir build -C Release --output-on-failure
  cmake --install build --config Release
  ```
- **Verify Core Without Modules**:
  ```powershell
  cmake -S . -B build-nomods -A x64 -DIRINE_MODULES=disabled
  cmake --build build-nomods --config Release --target DNGameServer DNVillageServer irine_tests --parallel
  ctest --test-dir build-nomods -C Release --output-on-failure
  ```
- **Linux Servers (x64)**:
  ```bash
  ./apps/compiler/compile.sh
  ```
- **Windows Client & Launcher (x64 default, `-Arch Win32` optional)**:
  ```powershell
  .\apps\compiler\compile-client.ps1
  ```

## Contribution Checklist

- Run `ctest` after server, common, module, or config template changes.
- Verify disabled-module compilation whenever changing core-to-module hooks or `src/cmake/IrineModules.cmake`.
- Verify PE architecture, imports, and Resource ID `1` UAC manifest after client/launcher build or resource changes.
- Test Docker MSSQL cold boot and warm restart on a disposable container/volume after touching `data/sql/base/` or `deploy/mssql/scripts/` without disturbing existing host containers.
- Clean up temporary build folders, scratch files, and test containers before finishing.
