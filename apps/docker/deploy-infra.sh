#!/usr/bin/env bash
# =============================================================================
# IrineCore — apps/docker/deploy-infra.sh
# Starts Dockerized MSSQL (auto-populates all 5 DBs directly from data/sql/base
# + data/sql/updates) and Nginx Patch WebServer, waiting until healthy.
# =============================================================================
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

for svc_dir in deploy/mssql deploy/webserver; do
    if [[ ! -f "${svc_dir}/.env" && -f "${svc_dir}/.env.example" ]]; then
        cp "${svc_dir}/.env.example" "${svc_dir}/.env"
        echo "[IrineCore] Created ${svc_dir}/.env from .env.example"
    fi
done

echo "[IrineCore] Building and starting irine-database and irine-webserver..."
docker compose up -d --build --wait irine-database irine-webserver

echo "[IrineCore] Infrastructure stack is populated and healthy."
echo "  - SQL Server  : 127.0.0.1,1433 (dngsm, dnmembership, dnserverlog, dnstaging, dnworld)"
echo "  - Patch HTTP  : http://127.0.0.1/Game/DragonNest/patch/PatchInfoServer.cfg"
echo "  - HealthCheck : http://127.0.0.1/health"
