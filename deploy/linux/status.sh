#!/usr/bin/env bash
# =============================================================================
# IrineCore — deploy/linux/status.sh
# Reports runtime status for all 7 IrineCore background daemons and SQL Server.
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
DIST_DIR="${ROOT_DIR}/env/dist"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --dist)
            DIST_DIR="$2"
            shift 2
            ;;
        *)
            DIST_DIR="$1"
            shift
            ;;
    esac
done

PID_DIR="${DIST_DIR}/run"
LOG_DIR="${DIST_DIR}/logs"

if (exec 3<>/dev/tcp/127.0.0.1/1433) 2>/dev/null; then
    exec 3>&-
    echo "SQL Server : UP (127.0.0.1:1433)"
else
    echo "SQL Server : DOWN (127.0.0.1:1433)"
fi
echo "Dist Dir   : ${DIST_DIR}"
echo ""

printf "%-20s %-10s %-10s %-10s %s\n" "SERVER" "STATE" "PID" "RSS_MB" "LOG"
printf "%-20s %-10s %-10s %-10s %s\n" "------" "-----" "---" "------" "---"

SERVERS=(
    "DNLogServer64"
    "DNDBServer64"
    "DNCashServer64"
    "DNMasterServer64"
    "DNLoginServer64"
    "DNVillageServer64"
    "DNGameServer64"
)

for srv in "${SERVERS[@]}"; do
    pid_file="${PID_DIR}/${srv}.pid"
    log_file="${LOG_DIR}/${srv}.log"
    pid=""
    if [[ -f "${pid_file}" ]]; then
        cand="$(cat "${pid_file}")"
        if [[ -n "${cand}" ]] && kill -0 "${cand}" 2>/dev/null; then
            pid="${cand}"
        fi
    fi
    if [[ -z "${pid}" ]] && command -v pgrep >/dev/null 2>&1; then
        pid="$(pgrep -x "${srv}" 2>/dev/null | head -n 1 || true)"
    fi

    if [[ -n "${pid}" ]]; then
        rss_kb="$(ps -o rss= -p "${pid}" 2>/dev/null | tr -d '[:space:]' || echo 0)"
        rss_mb="$(awk "BEGIN { printf \"%.1f\", ${rss_kb:-0}/1024 }")"
        printf "%-20s %-10s %-10s %-10s %s\n" "${srv}" "RUNNING" "${pid}" "${rss_mb}" "${log_file}"
    else
        printf "%-20s %-10s %-10s %-10s %s\n" "${srv}" "STOPPED" "-" "-" "${log_file}"
    fi
done
