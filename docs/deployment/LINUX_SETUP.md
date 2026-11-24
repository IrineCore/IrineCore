# IrineCore — Linux Setup & Operations Guide

Complete end-to-end guide for building, configuring (`GameRes`, ODBC, SQL Server, IPs/ports), and running all 7 **IrineCore** server daemons natively on Linux (`x86_64`).

---

## 1. Requirements

- **OS**: Ubuntu **22.04 / 24.04 / 26.04 LTS** or Debian **12 / 13** (`x86_64`) — native Linux bare metal, VPS, VM, or WSL2.
- **CPU / RAM / Disk**:
  - **Build**: 4+ CPU cores, 4+ GB RAM (8 GB recommended for parallel linking), ~10 GB free for source + `build/`.
  - **Runtime**: 4+ GB RAM (6–8 GB recommended when running SQL Server + all 7 servers on the same host), ~12 GB free for `GameRes` (~9.1 GB) + logs/backups.
- **Required packages** (installed automatically by `./irine.sh install`):
  - `build-essential`, `cmake` (>= 3.20), `ninja-build`, `zlib1g-dev`, `unixodbc`, `unixodbc-dev`, `curl`, `ca-certificates`, `gnupg`
  - **Microsoft ODBC Driver 18 for SQL Server** (`msodbcsql18`) and **SQL Server CLI tools** (`mssql-tools18`: `sqlcmd`, `bcp`)
  - Optional: `docker` + Docker Compose plugin (for containerized SQL Server 2022 + Nginx patch server), `python3` (for `./irine.sh db` and `./irine.sh module`), `ccache` (faster rebuilds).

---

## 2. Clone & Build (`./irine.sh install`)

```bash
git clone https://github.com/IrineCore/IrineCore.git
cd IrineCore
chmod +x irine.sh apps/*/*.sh deploy/linux/*.sh scripts/*.sh
```

Run the unified Linux installer:

```bash
./irine.sh install
```

What `./irine.sh install` does automatically:
1. Installs Linux build prerequisites via `apt-get` (`build-essential`, `cmake`, `ninja-build`, `unixodbc`, `unixodbc-dev`, `zlib1g-dev`).
2. Runs `deploy/linux/install-odbc.sh` to add the Microsoft package repository, install **Microsoft ODBC Driver 18 (`msodbcsql18`)** and **`mssql-tools18`** (`sqlcmd`, `bcp`), add `/opt/mssql-tools18/bin` to your shell `PATH`, and verify registration with `odbcinst -q -d`. *(Idempotent: already-installed packages are skipped.)*
3. Configures and builds all 7 servers, enabled static modules (`Map-Warp`, `Player-Bots`, `examples`), 4 CLI tools, and `irine_tests` in **Release (`x86_64`, `-fshort-wchar`)** using CMake + Ninja.
4. Runs `ctest --test-dir build --output-on-failure`.
5. Stages everything into `env/dist/` via `cmake --install`:
   ```
   env/dist/
   ├── DNLogServer64
   ├── DNDBServer64
   ├── DNCashServer64
   ├── DNLoginServer64
   ├── DNMasterServer64
   ├── DNVillageServer64
   ├── DNGameServer64
   ├── config/          (*.ini.dist always updated; *.ini seeded only if absent)
   ├── module/          (module *.ini.dist, *.ini, modules.manifest)
   ├── systems/         (ChannelInfo.xml)
   └── tools/           (DnMakeMD5, DnMakeFullPatch, PatchBuilder, DnClientCreator)
   ```

> **Useful installer options:**
> - Run `deploy/linux/install.sh` directly for the same flow with a formatted summary banner.
> - Skip the Microsoft ODBC step (e.g. build-only machine): `IRINE_SKIP_ODBC=1 ./irine.sh install`
> - Skip `sqlcmd`/`bcp` and install only the ODBC driver: `IRINE_SKIP_MSSQL_TOOLS=1 ./irine.sh install`
> - Persist custom CMake flags, parallel jobs, or disabled modules without editing tracked files:
>   ```bash
>   cp conf/dist/config.sh conf/config.sh
>   ```
> - Incremental rebuilds after code edits:
>   ```bash
>   ./irine.sh compiler build && ./irine.sh compiler test && ./irine.sh compiler install
>   ```

---

## 3. Setting Up `GameRes` (Required for 6 of 7 Servers)

`GameRes` contains the Dragon Nest Cap 60 server game data (~9.1 GB, ~115,000+ files: `.dnt` data tables, `.xml` UI/quest strings, `.lua` scripts, `.act` actions, `.msh`/`.skn`/collision/map grids). Because of its size and copyright, `GameRes` is **not** stored in the Git repository — you supply it from your Cap 60 server files.

### 3.1 Which servers need `GameRes`?

| Server | Reads `GameRes`? | Subdirectories used |
|---|---|---|
| `DNLogServer64` | No | — |
| `DNDBServer64` | **Yes** | `Resource/` (`ext/`, etc.) |
| `DNCashServer64` | **Yes** | `Resource/` (`ext/`, `uistring/`, etc.) |
| `DNLoginServer64` | **Yes** | `Resource/` (`ext/`, `uistring/`, etc.) |
| `DNMasterServer64` | **Yes** | `Resource/`, `MapData/` |
| `DNVillageServer64` | **Yes** | `Resource/` (`ext/`, `script/`, `uistring/`), `MapData/` |
| `DNGameServer64` | **Yes** | `Resource/` (all), `MapData/` (all) |

### 3.2 Expected `GameRes` Directory Structure

Your `GameRes` root directory **must** directly contain `Resource/` and `MapData/` (not nested inside an extra `GameRes/GameRes/` folder):

```text
GameRes/
├── MapData/
│   ├── grid/
│   └── resource/
└── Resource/
    ├── camera/
    ├── char/
    ├── dmv/
    ├── effect/
    ├── etc/
    ├── ext/                 <-- .dnt data tables (actortable.dnt, itemtable.dnt, ...)
    ├── fonts/
    ├── item/
    ├── movie/
    ├── particle/
    ├── script/              <-- QuestNPC_Common/, Talk_Npc/, Talk_Quest/ (.lua / .xml)
    ├── sharedeffect/
    ├── sound/
    ├── stateeffect/
    ├── ui/
    ├── uistring/            <-- uistring.xml (required at startup!)
    ├── uitemplatetexture/
    └── weapon/
```

Quick sanity check on your `GameRes` folder:
```bash
ls -la /path/to/GameRes
find /path/to/GameRes -maxdepth 3 -iname "uistring.xml"
```
You should see both `MapData` and `Resource`, and `uistring.xml` inside `Resource/uistring/`.

### 3.3 Copying `GameRes` from Windows to Linux

If your `GameRes` lives on a Windows machine and you are deploying to a Linux VPS/server:

- **Recommended (`rsync` over SSH — resumable, preserves structure):**
  ```bash
  # From WSL or Git Bash on Windows (adjust source path as needed):
  rsync -avh --progress /mnt/r/Server/GameRes/ user@your-linux-host:/srv/irinecore/GameRes/
  ```
- **Or archive & extract (faster when transferring 115k small files over high-latency links):**
  ```bash
  # On Windows / WSL:
  tar -C /mnt/r/Server -cf - GameRes | ssh user@your-linux-host 'mkdir -p /srv/irinecore && tar -C /srv/irinecore -xf -'
  ```
- **Permissions on Linux:** ensure the user running IrineCore can read and traverse every directory inside `GameRes`:
  ```bash
  sudo chown -R "$USER:$USER" /srv/irinecore/GameRes
  chmod -R a+rX /srv/irinecore/GameRes
  ```
- **WSL2 performance tip:** If running under WSL2 on Windows, keeping the repo and `GameRes` inside the native Linux `ext4` filesystem (e.g. `~/IrineCore` and `/srv/irinecore/GameRes`) instead of `/mnt/c` or `/mnt/d` makes C++ compilation and `GameRes` startup indexing dramatically faster (avoiding Windows 9p filesystem overhead across 115,000+ files).

### 3.4 Case Sensitivity on Linux (`ext4` / `xfs`)

Windows `GameRes` dumps have inconsistent casing (for example, `Resource/ext` and `Resource/uistring` in lowercase while server code requests `Resource\Ext` and `Resource\UIString`).

IrineCore handles this natively on Linux:
- `Irine::Platform::ResolveCaseInsensitivePath` normalizes `\` to `/` and resolves mismatched path casing automatically on `ext4`/`xfs` (with an O(1) `lstat` fast path whenever exact casing already matches).
- `CEtResourceMng`, `CFileStream`, `TinyXML`, and `Lua` all normalize Windows separators and preserve real on-disk casing during resource indexing.

You do **not** need to rename your 115,000 `GameRes` files to lowercase or uppercase — keep the directory as extracted, as long as there are no duplicate files differing only by case in the same directory.

### 3.5 Pointing IrineCore to `GameRes`

Choose whichever layout fits your deployment:

#### Option A — Place or symlink `GameRes` inside `env/dist/GameRes` (simplest for native Linux)

```bash
# Either move/copy GameRes into env/dist/GameRes, OR symlink your shared copy:
ln -s /srv/irinecore/GameRes env/dist/GameRes
```

Then update `[Resource] Path=` in all 6 active `.ini` files (the shipped templates default to `R:\Server\GameRes`):

```bash
for ini in DNDB.ini DNCash.ini DNLogin.ini DNMaster.ini DNVillage.ini DNGame.ini; do
    sed -i 's|^Path=.*|Path=./GameRes|' "env/dist/config/${ini}"
done
```

#### Option B — Use an absolute Linux path in `env/dist/config/*.ini`

Keep `GameRes` anywhere on disk (e.g. `/srv/irinecore/GameRes`) and set it directly:

```bash
GAMERES_DIR="/srv/irinecore/GameRes"
for ini in DNDB.ini DNCash.ini DNLogin.ini DNMaster.ini DNVillage.ini DNGame.ini; do
    sed -i "s|^Path=.*|Path=${GAMERES_DIR}|" "env/dist/config/${ini}"
done
```

> **Note:** Always edit `env/dist/config/*.ini` — **never** `*.ini.dist`. Running `./irine.sh compiler install` refreshes `*.ini.dist` with upstream defaults while preserving your local `*.ini` edits.

#### Option C — Full Docker Compose Stack

Copy `.env.example` to `.env` and set `IRINE_GAMERES_PATH`:

```bash
cp .env.example .env
# Edit .env:
#   IRINE_GAMERES_PATH=/srv/irinecore/GameRes
```

`docker-compose.yml` mounts `IRINE_GAMERES_PATH` read-only at `/irinecore/GameRes` into all 6 resource-reading containers, and `apps/docker/entrypoint.sh` automatically rewrites `[Resource] Path=` in a container-local runtime copy without modifying your host `env/dist/config/*.ini`.

---

## 4. Database Setup (SQL Server 2022 + ODBC)

IrineCore uses 5 Microsoft SQL Server databases: `dngsm`, `dnmembership`, `dnserverlog`, `dnstaging`, and `dnworld`, accessed via DSN-less **Microsoft ODBC Driver 18**.

### Option A — Automated Docker SQL Server + Nginx Patch Server (Recommended)

```bash
./irine.sh docker infra
```

This builds and starts `irine-database` (SQL Server 2022 on `127.0.0.1:1433`, auto-populated from `data/sql/base/` and `data/sql/updates/` with the `DragonNest` SQL login already created) and `irine-webserver` (Nginx patch server on port `80`).

Apply any new core or enabled-module SQL migrations:
```bash
./irine.sh db status
./irine.sh db update
```

### Option B — Existing / Native Linux SQL Server (`mssql-server`) or Remote MSSQL

1. Install Microsoft SQL Server for Linux or point to an existing SQL Server host on port `1433`.
2. Edit `env/dist/config/odbc.ini` (seeded from `odbc.ini.dist`):
   ```ini
   [ODBC]
   Driver=Auto
   Server=127.0.0.1
   Port=1433
   Trusted_Connection=No
   Uid=DragonNest
   Pwd=YourStrongPasswordHere
   Encrypt=Optional
   TrustServerCertificate=Yes
   LoginTimeout=5
   ConnectionTimeout=5

   [DNLog]
   Database=dnserverlog
   ```
   > **Critical for ODBC Driver 18 on Linux:** Driver 18 encrypts connections by default. Keep `TrustServerCertificate=Yes` unless your SQL Server has a CA-signed TLS certificate trusted by the OS.
3. Verify connectivity from Linux with `sqlcmd`:
   ```bash
   /opt/mssql-tools18/bin/sqlcmd -S 127.0.0.1,1433 -U DragonNest -P 'YourStrongPasswordHere' -C -Q "SELECT name FROM sys.databases"
   ```
4. Apply core + module updates:
   ```bash
   ./irine.sh db update
   ```

---

## 5. Network Ports, IPs & Firewall

### Server Port Map

| Server | Config File | Port(s) | Protocol | Exposure |
|---|---|---|---|---|
| `DNLogServer64` | `config/DNLog.ini` | `3000` | TCP | Internal (`127.0.0.1`) |
| `DNDBServer64` | `config/DNDB.ini` | `2600` | TCP | Internal (`127.0.0.1`) |
| `DNCashServer64` | `config/DNCash.ini` | `12600` | TCP | Internal (`127.0.0.1`) |
| `DNMasterServer64` | `config/DNMaster.ini` | `8000` (Village), `9000` (Game) | TCP | Internal (`127.0.0.1`) |
| `DNLoginServer64` | `config/DNLogin.ini` | `7000` (Master), **`14300`** (Client) | TCP | **`14300` Public** |
| `DNVillageServer64` | `config/DNVillage.ini` | **`14400`** (Client) | TCP | **`14400` Public** |
| `DNGameServer64` | `config/DNGame.ini` | **`500`** (TCP), **`5100–5101`** (UDP, `OpenCount=2`) | TCP + UDP | **Public** |
| SQL Server | `config/odbc.ini` | `1433` | TCP | Internal (`127.0.0.1`) |
| Patch WebServer | `deploy/webserver/` | `80` | TCP | Optional Public |

### Hosting on a LAN / Public VPS (Non-Localhost)

Keep all internal server-to-server links (`DB`, `Cash`, `Master`, `Log`, `odbc.ini`) on `127.0.0.1`, and set your public/LAN IP in these 3 places:
1. `env/dist/systems/ChannelInfo.xml`:
   ```xml
   <VillageServerInfo ID="1" IP="YOUR_PUBLIC_OR_LAN_IP" Port="14400">
   ```
2. `env/dist/config/DNVillage.ini`:
   ```ini
   [DolbyInfo]
   PrivateIp=YOUR_PUBLIC_OR_LAN_IP
   PublicIp=YOUR_PUBLIC_OR_LAN_IP
   ```
3. `env/dist/config/DNGame.ini`:
   ```ini
   [DolbyInfo]
   PrivateIp=YOUR_PUBLIC_OR_LAN_IP
   PublicIp=YOUR_PUBLIC_OR_LAN_IP
   ```
   *(Note: `DNGame.ini` defaults to `TcpPort=500`. On Linux, binding ports `< 1024` as a non-root user requires either `sudo setcap cap_net_bind_service=+ep env/dist/DNGameServer64`, running under `systemd` with `AmbientCapabilities=CAP_NET_BIND_SERVICE`, or changing `TcpPort` to an unprivileged port such as `15100`.)*

Example `ufw` firewall rules (expose only client-facing ports):
```bash
sudo ufw allow 14300/tcp        # DNLoginServer
sudo ufw allow 14400/tcp        # DNVillageServer
sudo ufw allow 500/tcp          # DNGameServer TCP (or your chosen TcpPort)
sudo ufw allow 5100:5109/udp    # DNGameServer UDP (StartPort .. StartPort + OpenCount)
```

---

## 6. Preflight Check & Starting the Servers

### 6.1 Run Preflight Diagnostics

```bash
./irine.sh doctor
```

Verify that Toolchain, ODBC Driver 18, `env/dist/config/*.ini`, `GameRes` (`Resource/` + `MapData/`), and SQL Server (`127.0.0.1:1433`) all report `[OK]`.

### 6.2 Start, Monitor, and Stop (Background Daemons)

```bash
# Start all 7 servers in dependency order (Log -> DB -> Cash -> Master -> Login -> Village -> Game)
./irine.sh server start

# Inspect running PIDs and uptime
./irine.sh server status

# Tail a server's stdout/stderr log
./irine.sh server logs DNLogServer64
./irine.sh server logs DNGameServer64

# Rotate logs (keeps last 5 backups)
./irine.sh server rotate-logs

# Gracefully stop (SIGTERM -> clean shutdown -> ClearApp) or restart
./irine.sh server stop
./irine.sh server restart
```

> **Logs location:**
> - Redirected process output (startup/daemon logs): `env/dist/logs/<ServerName>.log`
> - Daily structured server logs (`CLog`): `env/dist/log/<ServerName>_YYYY-MM-DD.log`
>
> When started detached (`< /dev/null`, `systemd`, `nohup`, or Docker), IrineCore automatically detects that stdin is not an interactive terminal, suppresses the `CMD>` prompt so logs never flood, and waits cleanly for `SIGTERM` / `SIGINT`.

### 6.3 Running a Single Server Interactively (Foreground Debugging)

Always run server binaries with `env/dist` as the working directory (they look up `./config/`, `./systems/`, `./module/`, and `./GameRes` relative to CWD):

```bash
cd env/dist
ODBCINI="$PWD/config/odbc.ini" ./DNLogServer64
```

When attached to a terminal, you get the interactive `CMD>` prompt and can type `exit` (or press `Ctrl+C`) to shut down cleanly.

### 6.4 Optional: Running Under `systemd`

Template units are provided in `deploy/linux/systemd/irine.target` and `deploy/linux/systemd/irine@.service`:

```bash
sudo cp deploy/linux/systemd/irine.target deploy/linux/systemd/irine@.service /etc/systemd/system/
# Edit User=, WorkingDirectory=, and ExecStart= in /etc/systemd/system/irine@.service if not using /opt/irinecore
sudo systemctl daemon-reload
for srv in DNLogServer64 DNDBServer64 DNCashServer64 DNMasterServer64 DNLoginServer64 DNVillageServer64 DNGameServer64; do
    sudo systemctl enable "irine@${srv}.service"
done
sudo systemctl start irine.target
```

---

## 7. Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `$'\r': command not found` or `set: pipefail\r: invalid option name` | Shell script saved with Windows CRLF endings | `sed -i 's/\r$//' irine.sh apps/*/*.sh deploy/linux/*.sh scripts/*.sh` |
| `Microsoft ODBC Driver 18` not found / `IM002` | `msodbcsql18` not installed or not registered | Run `bash deploy/linux/install-odbc.sh` and verify `odbcinst -q -d` |
| SQL login fails with TLS/SSL certificate error | ODBC Driver 18 defaults to encrypted TLS | Ensure `Encrypt=Optional` and `TrustServerCertificate=Yes` in `env/dist/config/odbc.ini` |
| `UI XML String File Load Failed : [uistring.xml]` | `[Resource] Path=` still points to `R:\Server\GameRes` or `GameRes` is missing `Resource/uistring/uistring.xml` | Verify `GameRes/Resource/uistring/uistring.xml` exists and update `Path=` in all 6 `env/dist/config/DN*.ini` files |
| `DNGameServer64` fails to bind TCP port `500` (`Permission denied`) | Linux restricts ports `< 1024` for non-root users | Run `sudo setcap cap_net_bind_service=+ep env/dist/DNGameServer64`, or change `TcpPort=` in `DNGame.ini` to `>= 1024` |
| `Too many open files` under load | Default Linux `ulimit -n` (1024) is too low | Set `ulimit -n 65535` (or configure `LimitNOFILE=65535` in systemd / `/etc/security/limits.conf`) |
| `DNLogServer64` exits immediately with `CreateDB Fail` | SQL Server is not running on `127.0.0.1:1433`, or `odbc.ini` credentials/database name mismatch | Start SQL Server (`./irine.sh docker infra`) and verify `[DNLog] Database=dnserverlog` in `env/dist/config/odbc.ini` |
