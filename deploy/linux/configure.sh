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

for ini in "${REQUIRED_SERVER_INIS[@]}"; do
    if [[ ! -f "${DIST_DIR}/config/${ini}" ]]; then
        echo "[ERROR] Missing required server configuration file: ${DIST_DIR}/config/${ini}" >&2
        exit 1
    fi
    echo "[OK] Found config/${ini}"
done

# Auto-discover enabled module configs from modules.manifest (generated at install time)
# or fall back to scanning modules/<Mod>/{modules,conf}/*.{ini,conf,conf.dist}.
MODULE_CONFIGS=()
MANIFEST_FILE="${DIST_DIR}/module/modules.manifest"
if [[ -f "${MANIFEST_FILE}" ]]; then
    while IFS= read -r line; do
        line="${line%$'\r'}"
        [[ -z "${line}" || "${line}" == \#* ]] && continue
        MODULE_CONFIGS+=("${line}")
    done < "${MANIFEST_FILE}"
elif [[ -d "${ROOT_DIR}/modules" ]]; then
    for mod_dir in "${ROOT_DIR}"/modules/*/; do
        [[ -d "${mod_dir}" ]] || continue
        mod_name="$(basename "${mod_dir}")"
        [[ "${mod_name}" == "disabled" || "${mod_name}" == .* ]] && continue
        [[ -f "${mod_dir}/.disabled" ]] && continue
        for sub in modules conf; do
            [[ -d "${mod_dir}/${sub}" ]] || continue
            for cfg in "${mod_dir}/${sub}"/*.ini "${mod_dir}/${sub}"/*.conf "${mod_dir}/${sub}"/*.conf.dist; do
                [[ -f "${cfg}" ]] && MODULE_CONFIGS+=("$(basename "${cfg}")")
            done
        done
    done
fi

for cfg in "${MODULE_CONFIGS[@]}"; do
    if [[ ! -f "${DIST_DIR}/module/${cfg}" ]]; then
        echo "[ERROR] Missing enabled module configuration file: ${DIST_DIR}/module/${cfg}" >&2
        exit 1
    fi
    echo "[OK] Found module/${cfg}"
done

if [[ ! -f "${DIST_DIR}/systems/ChannelInfo.xml" ]]; then
    echo "[ERROR] Missing required channel configuration: ${DIST_DIR}/systems/ChannelInfo.xml" >&2
    exit 1
fi

echo "[OK] Found systems/ChannelInfo.xml"
echo "[IrineCore] Linux configuration validation succeeded."
