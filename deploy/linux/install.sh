#!/usr/bin/env bash
# =============================================================================
# IrineCore — deploy/linux/install.sh
#
# Installs prerequisites:
#   Ubuntu 18.04, 20.04, 22.04, 24.04, 26.04
#   Debian 9, 10, 11, 12, 13
#
# Installs:
#   - Microsoft ODBC Driver 18                     (deploy/linux/install-odbc.sh)
#   - Microsoft SQL Server Tools 18 (sqlcmd, bcp)  (deploy/linux/install-odbc.sh)
#   - unixODBC development libraries
#   - GSSAPI dependency (Debian)
#   - CMake, Ninja, GCC/G++, and build dependencies
#
# Builds:
#   - IrineCore Release x86_64 (-fshort-wchar), runs CTest, installs to env/dist
#     via apps/compiler/compile.sh (honours conf/config.sh overrides)
#
# This is equivalent to `./irine.sh install` with an installation summary.
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

[[ -f "${ROOT_DIR}/conf/dist/config.sh" ]] && source "${ROOT_DIR}/conf/dist/config.sh"
[[ -f "${ROOT_DIR}/conf/config.sh" ]] && source "${ROOT_DIR}/conf/config.sh"

resolve_path() {
    case "$1" in
        /*) echo "$1" ;;
        *)  echo "${ROOT_DIR}/$1" ;;
    esac
}
BUILD_DIR="$(resolve_path "${IRINE_BUILD_DIR:-build}")"
DIST_DIR="$(resolve_path "${IRINE_DIST_DIR:-env/dist}")"

echo ""
echo "============================================================"
echo " IrineCore Linux Installer"
echo "============================================================"

if [[ ! -f /etc/os-release ]]; then
    echo "[ERROR] Cannot detect operating system." >&2
    exit 1
fi
# shellcheck disable=SC1091
. /etc/os-release
OS_ID="${ID}"
OS_VERSION="${VERSION_ID}"
echo "[IrineCore] Detected OS: ${OS_ID} ${OS_VERSION}"

# Dependencies + Microsoft ODBC Driver 18 + compile/test/install
bash "${ROOT_DIR}/apps/installer/install.sh" "$@"

# =============================================================================
# Verify Build Output
# =============================================================================
echo ""
echo "[IrineCore] Verifying build output..."
missing=0
for srv in DNLogServer64 DNDBServer64 DNCashServer64 DNLoginServer64 DNMasterServer64 DNVillageServer64 DNGameServer64; do
    if [[ -x "${DIST_DIR}/${srv}" ]]; then
        echo "[OK] ${DIST_DIR}/${srv}"
    else
        echo "[ERROR] Missing ${DIST_DIR}/${srv}" >&2
        missing=$((missing + 1))
    fi
done
if [[ "${missing}" -ne 0 ]]; then
    exit 1
fi

# =============================================================================
# Installation Summary
# =============================================================================
echo ""
echo "============================================================"
echo " IrineCore Installation Completed Successfully!"
echo "============================================================"
echo ""
echo " Operating System : ${OS_ID} ${OS_VERSION}"
echo " ODBC Driver      : Microsoft ODBC Driver 18"
echo " SQL Tools        : mssql-tools18 (/opt/mssql-tools18/bin)"
echo " Build Type       : ${IRINE_BUILD_TYPE:-Release}"
echo " Architecture     : x86_64"
echo " Compiler Flag    : -fshort-wchar"
echo " Build Directory  : ${BUILD_DIR}/"
echo " Server Binaries  : ${DIST_DIR}/"
echo ""
echo " Next steps       : docs/deployment/LINUX_SETUP.md"
echo "   1. Put GameRes in ${DIST_DIR}/GameRes (or set [Resource] Path=)"
echo "   2. Edit ${DIST_DIR}/config/*.ini (DB, IPs)"
echo "   3. ./irine.sh db update && ./irine.sh server start"
echo ""
echo "============================================================"
