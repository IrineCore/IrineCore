#!/usr/bin/env bash
# =============================================================================
# IrineCore — apps/docker/deploy-all.sh
# Builds & starts the complete 9-container IrineCore stack (auto-populated
# MSSQL from data/sql/base + Nginx + all 7 server daemons).
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

echo "[IrineCore] Building and starting complete 9-container cluster..."
docker compose up -d --build
docker compose ps
