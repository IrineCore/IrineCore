#!/usr/bin/env bash
# =============================================================================
# IrineCore — apps/docker/entrypoint.sh
# Container entrypoint launching the requested Dragon Nest server daemon
# directly from /irinecore/env/dist.
# =============================================================================
set -euo pipefail

DIST_DIR="/irinecore/env/dist"
SERVER_BIN="${1:-DNLoginServer64}"

cd "${DIST_DIR}"
export ODBCINI="${DIST_DIR}/config/odbc.ini"
if [[ -n "${IRINE_DB_HOST:-}" && -f "${DIST_DIR}/config/odbc.ini" ]]; then
    RUNTIME_ODBC="/tmp/irine_odbc.ini"
    sed -E "s|^Server=.*|Server=${IRINE_DB_HOST}|" "${DIST_DIR}/config/odbc.ini" > "${RUNTIME_ODBC}"
    export ODBCINI="${RUNTIME_ODBC}"
fi
export LD_LIBRARY_PATH="${DIST_DIR}:${LD_LIBRARY_PATH:-}"

if [[ -x "${DIST_DIR}/${SERVER_BIN}" ]]; then
    echo "[IrineCore] Starting ${SERVER_BIN} from ${DIST_DIR}..."
    exec "${DIST_DIR}/${SERVER_BIN}"
else
    echo "[IrineCore] Executing custom command: $*"
    exec "$@"
fi
