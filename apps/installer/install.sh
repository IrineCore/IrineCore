#!/usr/bin/env bash
# =============================================================================
# IrineCore — apps/installer/install.sh
# Bootstraps Linux dependencies and runs apps/compiler/compile.sh
# =============================================================================
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if command -v apt-get >/dev/null 2>&1; then
    sudo apt-get update -y
    sudo apt-get install -y build-essential cmake ninja-build unixodbc unixodbc-dev zlib1g-dev
fi
"${SCRIPT_DIR}/../compiler/compile.sh"
