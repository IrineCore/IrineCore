# Deployment & Operations Guide

## 0. Unified Management CLI (`irine.sh` & `irine.ps1`)

IrineCore provides a single top-level CLI on both Linux (`./irine.sh`) and Windows (`.\irine.ps1`) for building, testing, Docker deployment, database migrations, module management, log rotation, and preflight diagnostics:

```bash
# Linux / macOS / Git Bash
./irine.sh doctor                         # Preflight check (toolchain, ODBC, GameRes, configs, DB)
./irine.sh install                        # Install apt deps, compile, run CTest, stage env/dist
./irine.sh compiler build                 # Incremental build
./irine.sh compiler test                  # Run CTest suite
./irine.sh docker up                      # Launch full 9-container stack (MSSQL + Nginx + 7 servers)
./irine.sh db status                      # Show applied vs pending core & module SQL migrations
./irine.sh db update                      # Apply pending core + enabled-module SQL migrations
./irine.sh db backup                      # Create compressed .bak backups of all 5 databases
./irine.sh module list                    # List modules and enabled/disabled state
./irine.sh module enable Player-Bots      # Enable or disable a module without deleting code
./irine.sh module create My-Feature       # Scaffold a ready-to-build starter module
./irine.sh server rotate-logs             # Rotate native server logs (keeps 5 backups)
```

```powershell
# Windows PowerShell
.\irine.ps1 doctor
.\irine.ps1 compiler all                  # Configure, build x64 servers, run CTest, install env\dist
.\irine.ps1 compiler client               # Build Windows x64 DragonNest.exe & DnLauncher.exe
.\irine.ps1 docker infra                  # Start SQL Server 2022 + Nginx Patch WebServer
.\irine.ps1 db status
.\irine.ps1 db update
.\irine.ps1 module list
.\irine.ps1 server start -Background
```

### `.ini.dist` Config Layering & `conf/` Build Overrides
- **Runtime configs**: `cmake --install` always refreshes `env/dist/config/*.ini.dist` and `env/dist/module/*.ini.dist` with the latest defaults, while seeding active `*.ini` files **only if absent** so upgrades never overwrite your custom settings. At runtime, `CIniFile` reads from your local `*.ini` first and automatically falls back to `*.ini.dist` for any newly added keys.
- **Build & CLI overrides (`conf/`)**: Copy `conf/dist/config.sh` → `conf/config.sh` (Linux) or `conf\dist\config.ps1` → `conf\config.ps1` (Windows) to persist custom CMake flags, `IRINE_BUILD_TYPE`, disabled modules, `ccache`, or log retention settings without modifying tracked files.

## 1. Building Servers

### Windows (Visual Studio 2022/2026, x64)

Install the required Visual Studio 2022–2026 C++, ATL/MFC, CMake, vcpkg, and Windows SDK components using the repository's `.vsconfig` / `vs-requirements.json` file (Visual Studio auto-detects `.vsconfig` when opening the repo, or import via Visual Studio Installer UI / CLI):
```powershell
& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\setup.exe" modify `
    --installPath "<VS_INSTALL_PATH>" `
    --config ".\vs-requirements.json" --passive --norestart
```
*(Building the Windows client/launcher also requires the DirectX 9 SDK (June 2010).)*

```powershell
.\apps\compiler\compile.ps1
```
Or manually:
```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
cmake --install build --config Release
```

### Linux (Ubuntu 22.04–26.04 LTS / Debian 12+, x64)
> For the full step-by-step Linux walkthrough (including **`GameRes` setup**, ODBC Driver 18, SQL Server, ports/firewall, `systemd`, and troubleshooting), see [LINUX_SETUP.md](LINUX_SETUP.md).

```bash
./irine.sh install          # or: ./deploy/linux/install.sh
```
Or if dependencies and Microsoft ODBC Driver 18 are already installed:
```bash
./apps/compiler/compile.sh
```

`cmake --install` stages binaries, `config/` (from `apps/template/config/`), `systems/` (from `apps/template/System/`), and enabled `module/*.ini` files into `env/dist/`.

### Disabling Modules at Build Time
```powershell
# Exclude all modules
cmake -S . -B build -A x64 -DIRINE_MODULES=disabled

# Exclude specific modules
cmake -S . -B build -A x64 -DDISABLED_IRINE_MODULES="Player-Bots;examples"
```

## 2. Database & Patch WebServer

Start SQL Server 2022 (auto-populated directly from `data/sql/base/` + `data/sql/updates/`) and the Nginx patch server:

```powershell
.\apps\docker\deploy-infra.ps1
```
```bash
./apps/docker/deploy-infra.sh
```

Or run the entire 9-container stack (MSSQL + Nginx + all 7 Linux servers) via Docker Compose:
```bash
docker compose up -d --build
```

### Overriding `GameRes` Location in Docker

All 6 resource-reading server containers (`irine-dbserver`, `irine-cashserver`, `irine-masterserver`, `irine-loginserver`, `irine-villageserver`, `irine-gameserver`) mount `GameRes` read-only at `/irinecore/GameRes` by default, and `apps/docker/entrypoint.sh` automatically rewrites `[Resource] Path=` in a container-local runtime copy of `DN*.ini` without modifying your host `env/dist/config/*.ini` files.

You can override the host `GameRes` location in any of three ways:

1. **Via `.env` (recommended)** — copy `.env.example` to `.env` (gitignored) and set `IRINE_GAMERES_PATH`:
   ```dotenv
   # Linux
   IRINE_GAMERES_PATH=/srv/irinecore/GameRes
   # Windows
   IRINE_GAMERES_PATH=R:/Server/GameRes
   ```
2. **Via `docker-compose.override.yml`** — copy `docker-compose.override.yml.example` to `docker-compose.override.yml` (gitignored) and edit the host mount path under `x-gameres-override`.
3. **Inline per command**:
   ```bash
   IRINE_GAMERES_PATH=/srv/irinecore/GameRes docker compose up -d --build
   ```

To populate an existing local or remote SQL Server on Windows without Docker:
```powershell
.\deploy\mssql\scripts\populate-db.ps1 -Server "127.0.0.1,1433"
```

## 3. Launching Native Servers (Without Docker)

### Windows
```powershell
# Start all 7 servers in dependency order (add -Background for hidden background processes)
.\scripts\server.ps1 start
.\scripts\server.ps1 start -Background

# Check status, restart, or stop
.\scripts\server.ps1 status
.\scripts\server.ps1 restart
.\scripts\server.ps1 stop
```
*(Or call `deploy/windows/{start,stop,restart,status}.ps1` directly.)*

### Linux (Background Daemons)
```bash
# Start all 7 servers as detached background processes with PID files & logs under env/dist/
./scripts/server.sh start

# Check status, tail logs, restart, or stop
./scripts/server.sh status
./scripts/server.sh logs DNGameServer64
./scripts/server.sh restart
./scripts/server.sh stop
```
*(Or call `deploy/linux/{start,stop,restart,status}.sh`, or install the unit templates in `deploy/linux/systemd/`.)*

## 4. Building Windows Client & Launcher

```powershell
# Native x64 (default)
.\apps\compiler\compile-client.ps1

# Optional whole-program LTO
.\apps\compiler\compile-client.ps1 -LTO

# Legacy 32-bit Win32
.\apps\compiler\compile-client.ps1 -Arch Win32
```
Staged outputs in `env/client/`: `DragonNest.exe` and `DnLauncher.exe`.
