#!/usr/bin/env bash
# =============================================================================
# IrineCore — apps/compiler/compile.sh
# Configures, builds, runs CTest, and installs IrineCore into env/dist.
# =============================================================================
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# Load tracked defaults from conf/dist/config.sh, then optional local conf/config.sh
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
BUILD_TYPE="${IRINE_BUILD_TYPE:-Release}"

CMAKE_ARGS=(
    -S "${ROOT_DIR}"
    -B "${BUILD_DIR}"
    -G Ninja
    "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
    "-DCMAKE_INSTALL_PREFIX=${DIST_DIR}"
    "-DIRINE_MODULES=${IRINE_MODULES:-static}"
    "-DIRINE_BUILD_TOOLS=${IRINE_BUILD_TOOLS:-ON}"
)
if [[ -n "${DISABLED_IRINE_MODULES:-}" ]]; then
    CMAKE_ARGS+=("-DDISABLED_IRINE_MODULES=${DISABLED_IRINE_MODULES}")
fi
if [[ "${IRINE_USE_CCACHE:-ON}" == "ON" ]] && command -v ccache >/dev/null 2>&1; then
    CMAKE_ARGS+=("-DCMAKE_C_COMPILER_LAUNCHER=ccache" "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache")
fi
if [[ ${#IRINE_CMAKE_EXTRA_ARGS[@]} -gt 0 ]]; then
    CMAKE_ARGS+=("${IRINE_CMAKE_EXTRA_ARGS[@]}")
fi

cmake "${CMAKE_ARGS[@]}" "$@"

BUILD_ARGS=(--build "${BUILD_DIR}" --parallel)
if [[ -n "${IRINE_BUILD_JOBS:-}" ]]; then
    BUILD_ARGS=("--build" "${BUILD_DIR}" "--parallel" "${IRINE_BUILD_JOBS}")
fi
cmake "${BUILD_ARGS[@]}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure
cmake --install "${BUILD_DIR}"
