# Deployment & Operations Guide

## 1. Building Servers

### Windows (Visual Studio 2022/2026, x64)
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
```bash
./deploy/linux/install.sh
```
Or if dependencies are already installed:
```bash
./apps/compiler/compile.sh
```

`cmake --install` stages binaries, `config/` (from `apps/template/config/`), `systems/` (from `apps/template/System/`), and enabled `module/*.ini` files into `env/dist/`.

### Disabling Modules at Build Time
```powershell
# Exclude all modules
cmake -S . -B build -A x64 -DIRINE_MODULES=disabled

# Exclude specific modules
cmake -S . -B build -A x64 -DDISABLED_IRINE_MODULES="Player-Bots;Hero-Ally"
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
