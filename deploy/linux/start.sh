#!/usr/bin/env bash
# =============================================================================
# IrineCore — deploy/linux/start.sh
# Launches all 7 IrineCore servers as detached background daemons from env/dist
# in dependency order, writing PID files to <dist>/run and logs to <dist>/logs.
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
DIST_DIR="${ROOT_DIR}/env/dist"
STEP_DELAY=2
SKIP_DB_CHECK="${IRINE_SKIP_DB_CHECK:-0}"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --dist)
            DIST_DIR="$2"
            shift 2
            ;;
        --delay)
            STEP_DELAY="$2"
            shift 2
            ;;
        --skip-db-check)
            SKIP_DB_CHECK=1
            shift
            ;;
        *)
            DIST_DIR="$1"
            shift
            ;;
    esac
done

if [[ ! -d "${DIST_DIR}" ]]; then
    echo "[ERROR] Distribution directory not found: ${DIST_DIR}. Build and install servers first." >&2
    exit 1
fi

PID_DIR="${DIST_DIR}/run"
LOG_DIR="${DIST_DIR}/logs"
mkdir -p "${PID_DIR}" "${LOG_DIR}"

if [[ "${SKIP_DB_CHECK}" != "1" ]]; then
    if ! (exec 3<>/dev/tcp/127.0.0.1/1433) 2>/dev/null; then
        if command -v docker >/dev/null 2>&1; then
            echo "[IrineCore] SQL Server not detected on 127.0.0.1:1433. Starting irine-database + irine-webserver via Docker..."
            bash "${ROOT_DIR}/apps/docker/deploy-infra.sh"
        else
            echo "[WARN] SQL Server is not listening on 127.0.0.1:1433 and docker was not found." >&2
        fi
    else
        exec 3>&-
        echo "[OK] SQL Server reachable on 127.0.0.1:1433."
    fi
fi

SERVERS=(
    "DNLogServer64"
    "DNDBServer64"
    "DNCashServer64"
    "DNMasterServer64"
    "DNLoginServer64"
    "DNVillageServer64"
    "DNGameServer64"
)

cd "${DIST_DIR}"
export ODBCINI="${DIST_DIR}/config/odbc.ini"
export LD_LIBRARY_PATH="${DIST_DIR}:${LD_LIBRARY_PATH:-}"

for srv in "${SERVERS[@]}"; do
    bin="${DIST_DIR}/${srv}"
    if [[ ! -x "${bin}" ]]; then
        echo "[ERROR] Server binary not found or not executable: ${bin}" >&2
        exit 1
    fi

    pid_file="${PID_DIR}/${srv}.pid"
    if [[ -f "${pid_file}" ]]; then
        old_pid="$(cat "${pid_file}")"
        if [[ -n "${old_pid}" ]] && kill -0 "${old_pid}" 2>/dev/null; then
            echo "[SKIP] ${srv} is already running (PID ${old_pid})."
            continue
        fi
        rm -f "${pid_file}"
    fi

    log_file="${LOG_DIR}/${srv}.log"
    echo "[START] Launching ${srv} in background (log: ${log_file})..."
    if command -v setsid >/dev/null 2>&1; then
        setsid "${bin}" >> "${log_file}" 2>&1 < /dev/null &
    else
        nohup "${bin}" >> "${log_file}" 2>&1 < /dev/null &
    fi
    pid=$!
    echo "${pid}" > "${pid_file}"

    sleep "${STEP_DELAY}"
    if ! kill -0 "${pid}" 2>/dev/null; then
        echo "[ERROR] ${srv} exited immediately during startup. Last 20 lines of ${log_file}:" >&2
        tail -n 20 "${log_file}" >&2 || true
        rm -f "${pid_file}"
        exit 1
    fi
    echo "[OK] ${srv} running in background (PID ${pid})."
done

echo "[IrineCore] All 7 servers started in background. Use './scripts/server.sh status' to inspect."
