#!/usr/bin/env bash
# =============================================================================
# IrineCore — deploy/linux/restart.sh
# Restarts all 7 IrineCore background daemons cleanly on Linux.
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DIST_ARGS=()

for arg in "$@"; do
    case "${arg}" in
        --dist)
            ;;
    esac
done

# Extract --dist if passed so stop.sh targets the same distribution directory
PASS_DIST=""
args=("$@")
for ((i=0; i<${#args[@]}; i++)); do
    if [[ "${args[$i]}" == "--dist" && $((i+1)) -lt ${#args[@]} ]]; then
        PASS_DIST="${args[$((i+1))]}"
    fi
done

if [[ -n "${PASS_DIST}" ]]; then
    "${SCRIPT_DIR}/stop.sh" --dist "${PASS_DIST}"
else
    "${SCRIPT_DIR}/stop.sh"
fi

sleep 2
"${SCRIPT_DIR}/start.sh" "$@"
