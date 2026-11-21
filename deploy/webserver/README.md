# IrineCore — Patch & Launcher WebServer (`deploy/webserver/`)

Dockerized Nginx HTTP server serving Dragon Nest Cap 60 launcher configuration (`PatchConfigList.xml`, `PatchInfoServer.cfg`), launcher status page, and incremental `.pak` client patches.

## Contents
- `Dockerfile`: Builds an `nginx:1.27-alpine` image with patch-server Nginx rules and `/health` check.
- `docker-compose.yml`: Runs `irine-webserver` on port `${WEBSERVER_PORT:-80}` and bind-mounts `./www` read-only so manual patches take effect immediately without rebuilding.
- `nginx/default.conf`:
  - Case-insensitive `.MD5` / `.md5` lookup (`DNLauncher.exe` requests `.MD5` uppercase even when files on disk are lowercase `.md5`).
  - `Cache-Control: no-store` on `.xml`, `.cfg`, `.txt`, and `.md5` metadata files.
  - Native HTTP byte-range resume (`Range: bytes=...`) for `.pak` downloads.
  - `/health` endpoint and root fallback for `/PatchConfigList.xml` & `/PatchInfoServer.cfg`.
- `scripts/sync-patches.ps1` & `scripts/sync-patches.sh`: Sync versioned patch directories and `PatchInfoServer.cfg` into `www/Game/DragonNest/patch/`.
- `www/Game/DragonNest/patch/`: Holds `PatchConfigList.xml`, `PatchInfoServer.cfg`, and versioned patch directories (`00000354/`, `00000355/`, `00000356/`, ...).
- `www/Game/DragonNest/launcher/index.html`: Local launcher status page.

> **GitHub File Size Note:** Large binary `.pak` archives (e.g. `Patch00000355.pak`) are ignored by `.gitignore` to comply with GitHub's 100 MB file-size limit. Drop your local `.pak` files into `www/Game/DragonNest/patch/<version>/` or run `scripts/sync-patches.ps1` / `scripts/sync-patches.sh`.

## Quick Start

```bash
cd deploy/webserver
docker compose up -d --build
```

Verify endpoints:
```bash
curl -s http://127.0.0.1/health
curl -s http://127.0.0.1/Game/DragonNest/patch/PatchInfoServer.cfg
curl -I http://127.0.0.1/Game/DragonNest/patch/00000354/Patch00000354.pak.MD5
```
