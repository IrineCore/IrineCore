# MSSQL Docker & Database Initialization (`deploy/mssql/`)

SQL Server 2022 Docker setup that builds its initialization scripts directly from `data/sql/base/` at image build time (no duplicate SQL files under `deploy/mssql/`) and automatically populates all 5 Dragon Nest databases (`dngsm`, `dnmembership`, `dnserverlog`, `dnstaging`, `dnworld`) on first boot.

## How It Works
- `Dockerfile`: Multi-stage build using `python:3.12-alpine` + `scripts/build-sql-bundle.py` to assemble `data/sql/base/` into `/opt/dn-init/sql/` inside `mcr.microsoft.com/mssql/server:2022-latest`.
- `Dockerfile.dockerignore`: Limits Docker build context to `data/sql/base/` and the scripts in `deploy/mssql/scripts/`.
- `scripts/entrypoint.sh` & `scripts/init-db.sh`: Starts SQL Server, populates all 5 databases on an empty volume (or safely resumes an empty interrupted init), applies any mounted `data/sql/updates/*.sql` scripts on every boot, ensures the `DragonNest` login/roles and required seed rows exist, and unlocks the Docker healthcheck once verified.
- `scripts/populate-db.ps1`: Populates an existing local, remote, or containerized SQL Server directly from `data/sql/base/` using a temporary working directory.

## Usage

### Standalone Database Container
```bash
cd deploy/mssql
docker compose up -d --build --wait
```

### Database + Patch WebServer (for Native `env/dist` Hosting)
```powershell
.\apps\docker\deploy-infra.ps1
```
```bash
./apps/docker/deploy-infra.sh
```
*(Triggered automatically by the server launch scripts when port `1433` is not listening.)*

### Populate an Existing Local or Remote SQL Server
```powershell
.\deploy\mssql\scripts\populate-db.ps1 -Server "127.0.0.1,1433"
```

### Reset Databases from Scratch
```bash
cd deploy/mssql
docker compose down -v
docker compose up -d --build --wait
```
