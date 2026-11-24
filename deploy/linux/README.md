# Linux Deployment (`deploy/linux/`)

Supported distributions: **Ubuntu 22.04–26.04 LTS** and **Debian 12–13** (`x86_64`).

> **Full Setup & `GameRes` Guide:** See [docs/deployment/LINUX_SETUP.md](../../docs/deployment/LINUX_SETUP.md) for the complete walkthrough covering dependencies, `GameRes` structure and configuration, ODBC Driver 18 / SQL Server, ports/firewall, `systemd`, and troubleshooting.

- `install-odbc.sh`: Idempotent installer for Microsoft ODBC Driver 18 (`msodbcsql18`) and SQL Server CLI tools (`mssql-tools18`: `sqlcmd`, `bcp`). Automatically invoked by `./irine.sh install` and `deploy/linux/install.sh`.
- `install.sh` (or `./irine.sh install` / `apps/installer/install.sh`): Installs build prerequisites + Microsoft ODBC Driver 18, compiles all 7 servers, runs `ctest`, and installs to `env/dist/`.
- `configure.sh`: Validates `env/dist/config/*.ini`, `env/dist/module/*.ini`, and `env/dist/systems/ChannelInfo.xml`.
- `start.sh`: Launches all 7 servers from `env/dist/` as detached background daemons (`setsid`/`nohup`), writes PID files to `env/dist/run/`, and streams output to `env/dist/logs/<Server>.log`.
- `stop.sh` / `restart.sh` / `status.sh`: Gracefully stops (`SIGTERM` -> `SIGKILL`), restarts, or reports status of all background daemons.
- `systemd/`: Optional `irine.target` and `irine@.service` units for systemd-managed hosts.

You can invoke all actions from the repo root via `./irine.sh`:
```bash
./irine.sh install
./irine.sh doctor
./irine.sh server start
./irine.sh server status
./irine.sh server logs DNGameServer64
./irine.sh server stop
```
