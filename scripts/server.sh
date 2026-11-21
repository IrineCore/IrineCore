#!/usr/bin/env bash
# =============================================================================
# IrineCore — scripts/server.sh
# Convenience launcher for native Linux background server cluster management.
# Usage:
#   ./scripts/server.sh start [--dist <path>] [--delay <sec>] [--skip-db-check]
#   ./scripts/server.sh stop [--dist <path>]
#   ./scripts/server.sh restart [--dist <path>] [--delay <sec>] [--skip-db-check]
#   ./scripts/server.sh status [--dist <path>]
#   ./scripts/server.sh logs [ServerName] [--dist <path>]
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
LINUX_DIR="${ROOT_DIR}/deploy/linux"

ACTION="${1:-}"
if [[ -z "${ACTION}" ]]; then
    echo "Usage: $0 <start|stop|restart|status|logs [ServerName]> [options]" >&2
    exit 1
fi
shift

case "${ACTION}" in
    start)
        exec bash "${LINUX_DIR}/start.sh" "$@"
        ;;
    stop)
        exec bash "${LINUX_DIR}/stop.sh" "$@"
        ;;
    restart)
        exec bash "${LINUX_DIR}/restart.sh" "$@"
        ;;
    status)
        exec bash "${LINUX_DIR}/status.sh" "$@"
        ;;
    logs)
        SRV="${1:-DNGameServer64}"
        DIST_DIR="${ROOT_DIR}/env/dist"
        if [[ $# -ge 1 ]]; then shift; fi
        while [[ $# -gt 0 ]]; do
            case "$1" in
                --dist) DIST_DIR="$2"; shift 2 ;;
                *) shift ;;
            esac
        done
        LOG_FILE="${DIST_DIR}/logs/${SRV}.log"
        if [[ ! -f "${LOG_FILE}" ]]; then
            echo "[ERROR] Log file not found: ${LOG_FILE}" >&2
            exit 1
        fi
        exec tail -n 100 -f "${LOG_FILE}"
        ;;
    *)
        echo "Unknown action: ${ACTION}" >&2
        echo "Usage: $0 <start|stop|restart|status|logs [ServerName]> [options]" >&2
        exit 1
        ;;
esac
