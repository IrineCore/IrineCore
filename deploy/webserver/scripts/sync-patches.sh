#!/usr/bin/env bash
# =============================================================================
# IrineCore — deploy/webserver/scripts/sync-patches.sh
# Copies versioned patch folders and PatchInfoServer.cfg into deploy/webserver/www.
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR="${2:-${SCRIPT_DIR}/../www/Game/DragonNest/patch}"
SOURCE_DIR="${1:-}"

if [[ -z "${SOURCE_DIR}" || ! -d "${SOURCE_DIR}" ]]; then
    echo "Usage: $0 <source-patch-dir> [target-patch-dir]" >&2
    exit 1
fi

mkdir -p "${TARGET_DIR}"

for dir in "${SOURCE_DIR}"/*/; do
    [[ -d "${dir}" ]] || continue
    name="$(basename "${dir}")"
    mkdir -p "${TARGET_DIR}/${name}"
    cp -rf "${dir}"* "${TARGET_DIR}/${name}/"
    echo "[irine-webserver] Synced folder ${name}"
done

if [[ -f "${SOURCE_DIR}/PatchInfoServer.cfg" ]]; then
    cp -f "${SOURCE_DIR}/PatchInfoServer.cfg" "${TARGET_DIR}/PatchInfoServer.cfg"
    echo "[irine-webserver] Synced PatchInfoServer.cfg"
fi

echo "[irine-webserver] Patch sync complete -> ${TARGET_DIR}"
