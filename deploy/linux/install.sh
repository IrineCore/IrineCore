#!/usr/bin/env bash
# =============================================================================
# IrineCore — deploy/linux/install.sh
# Installs prerequisites (Ubuntu 22.04–26.04 LTS / Debian 12+) and builds IrineCore.
# =============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"

echo "[IrineCore] Checking Linux build & unixODBC prerequisites..."
if command -v apt-get >/dev/null 2>&1; then
    sudo apt-get update -y
    sudo apt-get install -y build-essential cmake ninja-build unixodbc unixodbc-dev zlib1g-dev
fi

echo "[IrineCore] Configuring CMake (Release x86_64 with -fshort-wchar)..."
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Release

echo "[IrineCore] Building all 7 IrineCore server binaries..."
cmake --build "${BUILD_DIR}" --parallel

echo "[IrineCore] Linux build completed: ${BUILD_DIR}/bin/"
