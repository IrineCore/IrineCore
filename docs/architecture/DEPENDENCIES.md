# Dependencies

IrineCore keeps ABI-coupled third-party libraries bundled under `deps/` and relies on system/SDK libraries for OS, database, and rendering APIs.

## Bundled Libraries (`deps/`)

| Library | Path | Purpose |
|---|---|---|
| **Lua 5.1.4 + tolua++** | `deps/lua51/` | NPC dialogue, quest, trigger, and monster AI scripting. Kept on Lua 5.1 for ABI compatibility with `tolua++` bindings (`game/characters/DnLuaAPIDefine.cpp`) and existing `.lua` scripts. |
| **TinyXML** | `deps/tinyxml/` | XML parsing (`TIXML_USE_STL`) for `ChannelInfo.xml`, `uistring.xml`, and launcher/quest metadata. |
| **zlib** | `deps/zlib/` | `.pak` archive decompression (`common/EtFileSystem/`) and stream compression. |
| **EtMathPortable** | `deps/etmath/` | Cross-platform C++ replacement for `d3dx9math.h` vector/matrix/quaternion types used by server collision, world grids, and headless 3D animation. |
| **Boost (headers)** | `deps/boost/` | Header-only subset (`smart_ptr`, `pool`, `algorithm/string`, `format`, `lexical_cast`, `tuple`, `crc`, `random`). |

## Server System Libraries

| Platform | Required Packages / SDKs |
|---|---|
| **Windows (x64)** | Visual Studio 2022/2026 C++ toolset, Windows SDK (`ws2_32`, `mswsock`, `odbc32`, `winmm`, `dbghelp`), ODBC Driver 18 for SQL Server (or SQL Server Native Client). |
| **Linux (x64)** | `build-essential` (GCC/G++), `cmake`, `ninja-build`, `unixodbc`, `unixodbc-dev`, `zlib1g-dev`, Microsoft `msodbcsql18` ODBC driver. |

## Client & Launcher Libraries (Windows Only)

- **Visual Studio C++ & Static MFC (`/MT`)**: Required for `DnLauncher.exe` (x64 default, Win32 supported).
- **DirectX 9 SDK (June 2010)**: `d3d9`, `d3dx9`, `dinput8`, `dsound`, `dxguid` (`Lib/x64` or `Lib/x86`), plus Windows SDK `ddraw.lib`.
- **Windows GDI+ (`gdiplus.lib`)**: Used by `DnLauncher.exe` for JPG/BMP resource decoding and rendering (no external image `.lib` dependencies).
- **FMOD & FreeType (bundled source)**: Compiled statically from source inside `src/client/engine/FMODEngine/FMOD_static/` and `src/client/engine/EtInterface/FreeType/` — no external `fmodex.dll` required.
- **Bandicam Video / Capture (`BVL` / `BCL`)**: Optional runtime `LoadLibrary` wrappers (`bdvid64.dll`/`bdcap64.dll` on x64, `bdvid32.dll`/`bdcap32.dll` on Win32) that degrade cleanly when absent.

## Per-Server Target Breakdown

All 7 servers link the shared static libraries from `common/` (`irine_platform`, `irine_utility`, `irine_etfilesystem`, `irine_etresmanager`, etc.) and compile role-specific sources from `src/server/ServerCommon/` and `game/` using their role macro (`_LOGSERVER`, `_DBSERVER`, `_CASHSERVER`, `_LOGINSERVER`, `_MASTERSERVER`, `_VILLAGESERVER`, `_GAMESERVER`).

Optional extensions in `modules/` register into `irine_modules` and `DNGameServer` via `src/cmake/IrineModules.cmake` only when enabled.
