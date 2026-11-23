#!/usr/bin/env bash
# =============================================================================
# IrineCore — Unified Cross-Platform Management CLI (irine.sh)
# =============================================================================
# Single entrypoint for building, testing, deploying, managing modules,
# running database migrations, rotating logs, and diagnosing the cluster.
# =============================================================================
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Load tracked defaults from conf/dist/config.sh, then optional local conf/config.sh
[[ -f "${ROOT_DIR}/conf/dist/config.sh" ]] && source "${ROOT_DIR}/conf/dist/config.sh"
[[ -f "${ROOT_DIR}/conf/config.sh" ]] && source "${ROOT_DIR}/conf/config.sh"

resolve_repo_path() {
    case "$1" in
        /*) echo "$1" ;;
        *)  echo "${ROOT_DIR}/$1" ;;
    esac
}

BUILD_DIR="$(resolve_repo_path "${IRINE_BUILD_DIR:-build}")"
DIST_DIR="$(resolve_repo_path "${IRINE_DIST_DIR:-env/dist}")"
BUILD_TYPE="${IRINE_BUILD_TYPE:-Release}"
detect_python() {
    for cand in python3 python; do
        if command -v "${cand}" >/dev/null 2>&1 && "${cand}" --version >/dev/null 2>&1; then
            command -v "${cand}"
            return 0
        fi
    done
    return 1
}
PYTHON_BIN="$(detect_python || true)"

print_help() {
    cat <<'EOF'
IrineCore — OpenSource Dragon Nest Server Emulator CLI

Usage: ./irine.sh <command> [subcommand] [options]

Commands:
  install                          Install Linux build prerequisites, compile, test, and stage env/dist
  compiler <configure|build|test|install|clean>
                                   Run CMake configure, parallel Ninja build, CTest, install, or clean
  server <start|stop|restart|status|logs [Name]|rotate-logs>
                                   Manage native Linux 7-server cluster daemons and log rotation
  docker <up|down|infra|build|pull|ps|logs [Service]|restart>
                                   Manage Docker Compose full cluster or infrastructure-only stack
  db <status|update|dry-run|backup|init> [args...]
                                   Run idempotent core + module SQL migrations and backups
  module <list|enable|disable|add|update|create|validate> [args...]
                                   Manage plug-and-play modules under modules/
  doctor                           Run preflight diagnostics (toolchain, ODBC, GameRes, configs, DB, ports)
  help                             Show this help message

Examples:
  ./irine.sh install
  ./irine.sh compiler build
  ./irine.sh docker up
  ./irine.sh db status
  ./irine.sh module list
  ./irine.sh module create My-Feature
  ./irine.sh doctor
EOF
}

require_python() {
    if [[ -z "${PYTHON_BIN}" ]]; then
        echo "[IrineCore][ERROR] python3 is required for this command." >&2
        exit 1
    fi
}

cmd_compiler() {
    local sub="${1:-build}"
    shift || true
    local cmake_args=(
        -S "${ROOT_DIR}"
        -B "${BUILD_DIR}"
        -G Ninja
        "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
        "-DCMAKE_INSTALL_PREFIX=${DIST_DIR}"
        "-DIRINE_MODULES=${IRINE_MODULES:-static}"
        "-DIRINE_BUILD_TOOLS=${IRINE_BUILD_TOOLS:-ON}"
    )
    [[ -n "${DISABLED_IRINE_MODULES:-}" ]] && cmake_args+=("-DDISABLED_IRINE_MODULES=${DISABLED_IRINE_MODULES}")
    if [[ "${IRINE_USE_CCACHE:-ON}" == "ON" ]] && command -v ccache >/dev/null 2>&1; then
        cmake_args+=("-DCMAKE_C_COMPILER_LAUNCHER=ccache" "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache")
    fi
    if [[ ${#IRINE_CMAKE_EXTRA_ARGS[@]} -gt 0 ]]; then
        cmake_args+=("${IRINE_CMAKE_EXTRA_ARGS[@]}")
    fi

    case "${sub}" in
        configure)
            cmake "${cmake_args[@]}" "$@"
            ;;
        build)
            if [[ ! -f "${BUILD_DIR}/build.ninja" && ! -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
                cmake "${cmake_args[@]}"
            fi
            if [[ -n "${IRINE_BUILD_JOBS:-}" ]]; then
                cmake --build "${BUILD_DIR}" --parallel "${IRINE_BUILD_JOBS}" "$@"
            else
                cmake --build "${BUILD_DIR}" --parallel "$@"
            fi
            ;;
        test)
            ctest --test-dir "${BUILD_DIR}" --output-on-failure "$@"
            ;;
        install)
            cmake --install "${BUILD_DIR}" "$@"
            ;;
        all)
            exec bash "${ROOT_DIR}/apps/compiler/compile.sh" "$@"
            ;;
        clean)
            rm -rf "${BUILD_DIR}"
            echo "[IrineCore] Cleaned build directory: ${BUILD_DIR}"
            ;;
        *)
            echo "[IrineCore][ERROR] Unknown compiler subcommand: ${sub}" >&2
            echo "Valid: configure | build | test | install | all | clean" >&2
            exit 1
            ;;
    esac
}

cmd_rotate_logs() {
    local log_dir="${DIST_DIR}/logs"
    local keep="${1:-${IRINE_LOG_ROTATE_KEEP:-5}}"
    if [[ ! -d "${log_dir}" ]]; then
        echo "[IrineCore] No log directory at ${log_dir}."
        return 0
    fi
    local rotated=0
    for lf in "${log_dir}"/*.log; do
        [[ -f "${lf}" ]] || continue
        local sz
        sz="$(wc -c < "${lf}" 2>/dev/null || echo 0)"
        if (( sz == 0 )); then continue; fi
        for (( i = keep - 1; i >= 1; i-- )); do
            [[ -f "${lf}.${i}" ]] && mv -f "${lf}.${i}" "${lf}.$((i + 1))"
        done
        cp "${lf}" "${lf}.1"
        : > "${lf}"
        echo "[IrineCore] Rotated $(basename "${lf}") (${sz} bytes)"
        rotated=$((rotated + 1))
    done
    echo "[IrineCore] Log rotation complete (${rotated} file(s) rotated)."
}

cmd_server() {
    local sub="${1:-status}"
    shift || true
    if [[ "${sub}" == "rotate-logs" ]]; then
        cmd_rotate_logs "$@"
        return 0
    fi
    exec bash "${ROOT_DIR}/scripts/server.sh" "${sub}" "$@"
}

cmd_docker() {
    local sub="${1:-ps}"
    shift || true
    cd "${ROOT_DIR}"
    case "${sub}" in
        up)      docker compose up -d --build "$@" ;;
        down)    docker compose down "$@" ;;
        infra)   exec bash "${ROOT_DIR}/apps/docker/deploy-infra.sh" "$@" ;;
        build)   docker compose build "$@" ;;
        pull)    docker compose pull "$@" ;;
        ps)      docker compose ps "$@" ;;
        logs)    docker compose logs -f --tail=100 "$@" ;;
        restart) docker compose restart "$@" ;;
        *)
            echo "[IrineCore][ERROR] Unknown docker subcommand: ${sub}" >&2
            echo "Valid: up | down | infra | build | pull | ps | logs | restart" >&2
            exit 1
            ;;
    esac
}

cmd_db() {
    require_python
    local sub="${1:-status}"
    shift || true
    if [[ "${sub}" == "dry-run" ]]; then
        exec "${PYTHON_BIN}" "${ROOT_DIR}/apps/db-assembler/db_assembler.py" update --dry-run "$@"
    fi
    exec "${PYTHON_BIN}" "${ROOT_DIR}/apps/db-assembler/db_assembler.py" "${sub}" "$@"
}

cmd_module() {
    require_python
    local sub="${1:-list}"
    shift || true
    exec "${PYTHON_BIN}" "${ROOT_DIR}/apps/module-manager/module_manager.py" "${sub}" "$@"
}

cmd_doctor() {
    local warns=0
    local fails=0
    ok()   { echo "  [OK]   $*"; }
    warn() { echo "  [WARN] $*"; warns=$((warns + 1)); }
    fail() { echo "  [FAIL] $*"; fails=$((fails + 1)); }

    echo "=== IrineCore Preflight Diagnostics ==="
    echo "[1/5] Toolchain"
    for tool in cmake git; do
        if command -v "${tool}" >/dev/null 2>&1; then ok "${tool} ($(command -v "${tool}"))"; else fail "Missing ${tool}"; fi
    done
    for tool in ninja g++ ccache docker python3; do
        if command -v "${tool}" >/dev/null 2>&1; then ok "${tool} available"; else warn "Optional tool '${tool}' not found"; fi
    done

    echo "[2/5] ODBC Driver"
    if command -v odbcinst >/dev/null 2>&1; then
        if odbcinst -q -d 2>/dev/null | grep -qi "ODBC Driver.*SQL Server"; then
            ok "Microsoft SQL Server ODBC Driver installed"
        else
            warn "unixODBC found, but Microsoft ODBC Driver 18 (msodbcsql18) is not registered"
        fi
    else
        warn "odbcinst not installed (required for native Linux SQL Server connections)"
    fi

    echo "[3/5] Staged Distribution (env/dist)"
    if [[ -d "${DIST_DIR}" ]]; then
        for ini in DNLog.ini DNDB.ini DNCash.ini DNLogin.ini DNMaster.ini DNVillage.ini DNGame.ini odbc.ini; do
            if [[ -f "${DIST_DIR}/config/${ini}" || -f "${DIST_DIR}/config/${ini}.dist" ]]; then
                ok "config/${ini}"
            else
                warn "Missing ${DIST_DIR}/config/${ini} (run ./irine.sh compiler install)"
            fi
        done
        if [[ -f "${DIST_DIR}/systems/ChannelInfo.xml" ]]; then ok "systems/ChannelInfo.xml"; else warn "Missing systems/ChannelInfo.xml"; fi
        if [[ -f "${DIST_DIR}/module/modules.manifest" ]]; then ok "module/modules.manifest"; else warn "Missing module/modules.manifest"; fi
    else
        warn "env/dist not staged yet (run ./irine.sh install or ./irine.sh compiler all)"
    fi

    echo "[4/5] GameRes Path"
    local gameres_env=""
    if [[ -f "${ROOT_DIR}/.env" ]]; then
        gameres_env="$(grep -E '^IRINE_GAMERES_PATH=' "${ROOT_DIR}/.env" | tail -n1 | cut -d= -f2- | tr -d '\r"'\''' || true)"
    fi
    local gameres_check="${IRINE_GAMERES_PATH:-${gameres_env:-${ROOT_DIR}/env/GameRes}}"
    if [[ -d "${gameres_check}" ]] && [[ -n "$(ls -A "${gameres_check}" 2>/dev/null)" ]]; then
        ok "GameRes found and non-empty: ${gameres_check}"
    else
        warn "GameRes directory missing or empty at '${gameres_check}' (set IRINE_GAMERES_PATH in .env)"
    fi

    echo "[5/5] SQL Server Connectivity (127.0.0.1:1433)"
    if (echo >/dev/tcp/127.0.0.1/1433) >/dev/null 2>&1; then
        ok "SQL Server port 1433 is reachable on 127.0.0.1"
    else
        warn "SQL Server port 1433 not reachable on 127.0.0.1 (start via ./irine.sh docker infra)"
    fi

    echo "=== Summary: ${fails} error(s), ${warns} warning(s) ==="
    (( fails == 0 ))
}

ACTION="${1:-help}"
shift || true

case "${ACTION}" in
    install)    exec bash "${ROOT_DIR}/apps/installer/install.sh" "$@" ;;
    compiler)   cmd_compiler "$@" ;;
    server)     cmd_server "$@" ;;
    docker)     cmd_docker "$@" ;;
    db)         cmd_db "$@" ;;
    module)     cmd_module "$@" ;;
    doctor)     cmd_doctor "$@" ;;
    help|-h|--help) print_help ;;
    *)
        echo "[IrineCore][ERROR] Unknown command: ${ACTION}" >&2
        print_help >&2
        exit 1
        ;;
esac
