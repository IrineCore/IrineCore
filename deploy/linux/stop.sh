#!/usr/bin/env bash
# =============================================================================
# IrineCore — deploy/linux/stop.sh
# Gracefully stops all 7 IrineCore background daemons in reverse dependency order.
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
DIST_DIR="${ROOT_DIR}/env/dist"
GRACEFUL_TIMEOUT=10

while [[ $# -gt 0 ]]; do
    case "$1" in
        --dist)
            DIST_DIR="$2"
            shift 2
            ;;
        --timeout)
            GRACEFUL_TIMEOUT="$2"
            shift 2
            ;;
        *)
            DIST_DIR="$1"
            shift
            ;;
    esac
done

PID_DIR="${DIST_DIR}/run"

SERVERS_REVERSE=(
    "DNGameServer64"
    "DNVillageServer64"
    "DNLoginServer64"
    "DNMasterServer64"
    "DNCashServer64"
    "DNDBServer64"
    "DNLogServer64"
)

stop_pid() {
    local srv="$1"
    local pid="$2"
    if ! kill -0 "${pid}" 2>/dev/null; then
        return 0
    fi
    echo "[STOP] Sending SIGTERM to ${srv} (PID ${pid})..."
    kill -TERM "${pid}" 2>/dev/null || true

    local waited=0
    while kill -0 "${pid}" 2>/dev/null && [[ "${waited}" -lt "${GRACEFUL_TIMEOUT}" ]]; do
        sleep 1
        waited=$((waited + 1))
    done

    if kill -0 "${pid}" 2>/dev/null; then
        echo "[WARN] ${srv} (PID ${pid}) did not exit after ${GRACEFUL_TIMEOUT}s; sending SIGKILL..."
        kill -KILL "${pid}" 2>/dev/null || true
    fi
}

for srv in "${SERVERS_REVERSE[@]}"; do
    pid_file="${PID_DIR}/${srv}.pid"
    stopped=0
    if [[ -f "${pid_file}" ]]; then
        pid="$(cat "${pid_file}")"
        if [[ -n "${pid}" ]] && kill -0 "${pid}" 2>/dev/null; then
            stop_pid "${srv}" "${pid}"
            stopped=1
        fi
        rm -f "${pid_file}"
    fi

    if [[ "${stopped}" -eq 0 ]] && command -v pgrep >/dev/null 2>&1; then
        for pid in $(pgrep -x "${srv}" 2>/dev/null || true); do
            stop_pid "${srv}" "${pid}"
        done
    fi
done

echo "[IrineCore] All servers stopped."
