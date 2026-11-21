# Windows Deployment (`deploy/windows/`)

- `install.ps1` (or `apps/compiler/compile.ps1`): Configures CMake (`x64`), builds all 7 servers, runs `ctest`, and installs to `env/dist/`.
- `configure.ps1`: Checks `env/dist/config/*.ini`, `env/dist/module/*.ini`, and `env/dist/systems/ChannelInfo.xml`.
- `start.ps1`: Starts all 7 servers from `env/dist/` in dependency order. Pass `-Background` to run hidden background processes with logs under `env/dist/logs/`, or `-SkipDbCheck` to bypass the port `1433` check.
- `stop.ps1` / `restart.ps1` / `status.ps1`: Stops, restarts, or reports live status of the cluster.

You can also invoke all actions from the repo root via `scripts/server.ps1`:
```powershell
.\scripts\server.ps1 start -Background
.\scripts\server.ps1 status
.\scripts\server.ps1 stop
```
