#!/usr/bin/env bash
# =============================================================================
# IrineCore — deploy/linux/configure.sh
# Validates env/dist/config/*.ini, env/dist/module/*.ini, and env/dist/systems/ChannelInfo.xml.
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
DIST_DIR="${1:-${ROOT_DIR}/env/dist}"

REQUIRED_SERVER_INIS=(
    "DNLog.ini" "DNDB.ini" "DNCash.ini" "DNLogin.ini" "DNMaster.ini"
    "DNVillage.ini" "DNGame.ini" "odbc.ini"
)

REQUIRED_MODULE_INIS=(
    "Player-Bots.ini" "Hero-Ally.ini" "FPS-PING.ini"
)

for ini in "${REQUIRED_SERVER_INIS[@]}"; do
    if [[ ! -f "${DIST_DIR}/config/${ini}" ]]; then
        echo "[ERROR] Missing required server configuration file: ${DIST_DIR}/config/${ini}" >&2
        exit 1
    fi
    echo "[OK] Found config/${ini}"
done

for ini in "${REQUIRED_MODULE_INIS[@]}"; do
    if [[ ! -f "${DIST_DIR}/module/${ini}" ]]; then
        echo "[ERROR] Missing required module configuration file: ${DIST_DIR}/module/${ini}" >&2
        exit 1
    fi
    echo "[OK] Found module/${ini}"
done

if [[ ! -f "${DIST_DIR}/systems/ChannelInfo.xml" ]]; then
    echo "[ERROR] Missing required channel configuration: ${DIST_DIR}/systems/ChannelInfo.xml" >&2
    exit 1
fi

echo "[OK] Found systems/ChannelInfo.xml"
echo "[IrineCore] Linux configuration validation succeeded."
