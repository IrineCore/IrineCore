# Linux Deployment (`deploy/linux/`)

Supported distributions: **Ubuntu 22.04–26.04 LTS** and **Debian 12+**.

- `install.sh` (or `apps/installer/install.sh`): Installs build dependencies, compiles all 7 servers, runs `ctest`, and installs to `env/dist/`.
- `configure.sh`: Validates `env/dist/config/*.ini`, `env/dist/module/*.ini`, and `env/dist/systems/ChannelInfo.xml`.
- `start.sh`: Launches all 7 servers from `env/dist/` as detached background daemons (`setsid`/`nohup`), writes PID files to `env/dist/run/`, and streams output to `env/dist/logs/<Server>.log`.
- `stop.sh` / `restart.sh` / `status.sh`: Gracefully stops (`SIGTERM` -> `SIGKILL`), restarts, or reports status of all background daemons.
- `systemd/`: Optional `irine.target` and `irine@.service` units for systemd-managed hosts.

You can also invoke all actions from the repo root via `scripts/server.sh`:
```bash
./scripts/server.sh start
./scripts/server.sh status
./scripts/server.sh logs DNGameServer64
./scripts/server.sh stop
```
