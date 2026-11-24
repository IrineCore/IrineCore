#!/usr/bin/env bash
# =============================================================================
# IrineCore — apps/installer/install.sh   (invoked by: ./irine.sh install)
#
# 1. Installs Linux build dependencies (GCC, CMake, Ninja, unixODBC, zlib)
# 2. Installs Microsoft ODBC Driver 18 + sqlcmd/bcp (deploy/linux/install-odbc.sh)
# 3. Runs apps/compiler/compile.sh: configure, build, CTest, install to env/dist
#
# Environment:
#   IRINE_SKIP_ODBC=1         Skip the Microsoft ODBC Driver 18 step
#   IRINE_SKIP_MSSQL_TOOLS=1  Install the ODBC driver without sqlcmd/bcp
# Extra arguments are forwarded to the CMake configure step.
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

if command -v apt-get >/dev/null 2>&1; then
    if [[ "$(id -u)" -eq 0 ]]; then SUDO=(); else SUDO=(sudo); fi

    echo "[IrineCore] Installing Linux build prerequisites..."
    "${SUDO[@]}" apt-get update -y
    "${SUDO[@]}" apt-get install -y \
        build-essential cmake ninja-build \
        unixodbc unixodbc-dev zlib1g-dev \
        curl ca-certificates gnupg

    bash "${ROOT_DIR}/deploy/linux/install-odbc.sh"
else
    echo "[IrineCore][WARN] apt-get not found. Install GCC/G++, CMake >= 3.20, Ninja, unixODBC (+dev)," >&2
    echo "                  zlib (+dev) and Microsoft ODBC Driver 18 (msodbcsql18) manually." >&2
fi

bash "${ROOT_DIR}/apps/compiler/compile.sh" "$@"

echo ""
echo "[IrineCore] Install complete. Servers staged in: ${IRINE_DIST_DIR:-env/dist}"
echo "[IrineCore] Next: see docs/deployment/LINUX_SETUP.md (GameRes, configs, database, start)."
