# Windows Client & Launcher (`src/client/`)

`DragonNest.exe` and `DnLauncher.exe` are Windows targets supporting native **x64** (default) and **Win32**.

## Details
- **Native x64**: Removes 32-bit virtual address space limits in crowded hubs/nests and links directly against DirectX 9 SDK `Lib/x64` and Static MFC x64 (`/MT`).
- **Native GDI+ Launcher**: `DnLauncher` decodes JPG/BMP skin resources with Windows GDI+ (`gdiplus.lib`), removing legacy 32-bit `CxImage` `.lib` dependencies.
- **Static FMOD & FreeType**: Compiled directly from source (`engine/FMODEngine/FMOD_static/`, `engine/EtInterface/FreeType/`) with no external `fmodex.dll`.
- **UAC Elevation**: Both executables embed a single Resource ID `1` manifest requesting `requireAdministrator` (`uiAccess="false"`).
- **Build Flags**: Release builds use `/O2 /Oi /Ot /GF /Gy`, `/OPT:REF /OPT:ICF /INCREMENTAL:NO`, and `/DYNAMICBASE /NXCOMPAT /HIGHENTROPYVA` (optional `/GL` + `/LTCG` via `-LTO`).

## Prerequisites
1. Visual Studio 2022/2026 with **Desktop development with C++**, **MSVC x64 build tools**, and **C++ MFC for x64 build tools** (plus x86 tools for `-Arch Win32`).
2. **Microsoft DirectX SDK (June 2010)** with `DXSDK_DIR` set (or pass `-DIRINE_DXSDK_DIR=<path>` to CMake).

## Build & Install (`env/client/`)
```powershell
# Native x64 Release (default)
.\apps\compiler\compile-client.ps1

# Whole-program LTO x64 Release
.\apps\compiler\compile-client.ps1 -LTO

# Legacy Win32 Release
.\apps\compiler\compile-client.ps1 -Arch Win32
```

Installed outputs: `env/client/DnLauncher.exe` and `env/client/DragonNest.exe`.
