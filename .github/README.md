# ![logo](https://raw.githubusercontent.com/monakaibrahim-cmyk/IrineCore/refs/heads/master/.github/images/logo.png) IrineCore

[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](CODE_OF_CONDUCT.md)
[![Discord](https://img.shields.io/badge/Discord-Join%20Server-5865F2?logo=discord&logoColor=white)](https://discord.gg/zPdNnFEyPv)

## Build Status

[![docker-build](https://github.com/monakaibrahim-cmyk/IrineCore/actions/workflows/docker_build.yml/badge.svg?branch=master)](https://github.com/monakaibrahim-cmyk/IrineCore/actions/workflows/docker_build.yml?query=branch%3Amaster)
[![windows-build](https://github.com/monakaibrahim-cmyk/IrineCore/actions/workflows/windows_build.yml/badge.svg?branch=master)](https://github.com/monakaibrahim-cmyk/IrineCore/actions/workflows/windows_build.yml?query=branch%3Amaster)
[![linux-build](https://github.com/monakaibrahim-cmyk/IrineCore/actions/workflows/linux_build.yml/badge.svg?branch=master)](https://github.com/monakaibrahim-cmyk/IrineCore/actions/workflows/linux_build.yml?query=branch%3Amaster)


## Introduction

**IrineCore** is a modular, cross-platform (**Windows + Linux**) C++ open-source **Dragon Nest MMORPG Server Emulator** (Cap 60).

## Philosophy

Our main goal is to create a playable game server, offering a fully working in-game experience.

Here are the main points we focus on:

* Stability
  * We make sure all changes pass the CIs before being merged into the master branch.

* official content
  * We strive to make all in-game content to be official. Therefore we have a high standard for fixes being made.

* Customization
  * It is easy to customize your experience using [modules](#modules).

### Modules

IrineCore isolates gameplay extensions as standalone static libraries under `modules/`. Each module contains its own `CMakeLists.txt`, `src/`, default configuration template under `modules/<ModuleName>/modules/<ModuleName>.ini` (installed into `env/dist/module/`), and optional `resource/` and `data/` folders:

- **`Player-Bots`**: Autonomous AI party companions for Hard, Master, and Abyss dungeons and Nests (excluded on Easy and Normal stages).

## Installation

### 1. Start MSSQL (Auto-Init 5 Databases) & Patch WebServer
```powershell
# Windows
.\apps\docker\deploy-infra.ps1
```
```bash
# Linux
./apps/docker/deploy-infra.sh
```

### 2. Build & Install Server Cluster to `env/dist`

**Windows (`x64`):**
```powershell
.\apps\compiler\compile.ps1
.\scripts\server.ps1 start
```

**Linux (`Ubuntu 22.04–26.04 LTS` / `Debian 12+`):**
```bash
./apps/compiler/compile.sh
./scripts/server.sh start
```

**Full Docker Compose Cluster (MSSQL + Patch WebServer + 7 Game Servers):**
```bash
docker compose up -d --build
```

### 3. Build Windows-Only Client & Launcher (`src/client/`)
> `DragonNest.exe` and `DnLauncher.exe` are **Windows** targets supporting native **x64** (default) and legacy **Win32**, requiring Visual Studio (C++ + Static MFC toolset) and Microsoft DirectX SDK (June 2010).

```powershell
.\apps\compiler\compile-client.ps1
```
Staged outputs in `env/client/`: `DragonNest.exe` and `DnLauncher.exe` (both native x64 with Administrator UAC manifest).

### Distribution Layout (`env/dist/`)
```text
env/dist/
├── config/            # Server .ini & .dat files
├── module/            # Auto-discovered module configs + modules.manifest
├── systems/           # ChannelInfo.xml
├── tools/             # DnClientCreator, PatchBuilder, DnMakeFullPatch, DnMakeMD5
├── <server-binaries>  # DNLogServer64 .. DNGameServer64
└── <dlls>             # Runtime dynamic libraries (if any)
```

### Packaging & Patching Tools (`tools/`)
See [`tools/README.md`](../tools/README.md) for usage examples of `DnClientCreator` (packs unpacked `GameRes` folders into split `ResourceXX.pak` client builds), `PatchBuilder`, `DnMakeFullPatch`, and `DnMakeMD5`.

## Community & Support

Join the IrineCore Discord server for development discussion, build help, and community modules:

- **Discord**: [discord.gg/zPdNnFEyPv](https://discord.gg/zPdNnFEyPv)

## Contributing

IrineCore can also serve as a learning resource for aspiring developers who want to understand how DragonNest servers work, how MMORPGs are structured, how game server emulators are created, or to improve their C++ and SQL knowledge.

## Authors & Contributors

The project was born in 2026 based on Leaked Source Code.

## License

- The IrineCore source code is released under the [GNU GPL v2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

It's important to note that IrineCore is not an official Eyedentity Games product, and it is not affiliated with or endorsed by DragonNest or Eyedentity Games. IrineCore does not in any case sponsor nor support illegal public servers. If you use this project to run an illegal public server and not for testing and learning it is your own personal choice.