#!/usr/bin/env bash
# =============================================================================
# IrineCore — apps/docker/entrypoint.sh
# Container entrypoint launching the requested Dragon Nest server daemon from
# an isolated runtime directory so container-specific overrides (GameRes path,
# Docker service hostnames, PublicIp, DB host/credentials) never mutate mounted
# host env/dist/config/*.ini files.
# =============================================================================
set -euo pipefail

DIST_DIR="/irinecore/env/dist"
RUN_DIR="/tmp/irine-run"
SERVER_BIN="${1:-DNLoginServer64}"
GAMERES_PATH="${IRINE_GAMERES_CONTAINER_PATH:-/irinecore/GameRes}"
LOG_MAX_MB="${IRINE_LOG_MAX_MB:-50}"

# Rotate daemon log on startup if it exceeds IRINE_LOG_MAX_MB (keeps 5 backups)
rotate_log_if_needed() {
    local log_file="$1"
    local max_bytes=$(( LOG_MAX_MB * 1024 * 1024 ))
    if [[ -f "${log_file}" ]]; then
        local sz
        sz="$(wc -c < "${log_file}" 2>/dev/null || echo 0)"
        if (( sz >= max_bytes )); then
            for i in 4 3 2 1; do
                [[ -f "${log_file}.${i}" ]] && mv -f "${log_file}.${i}" "${log_file}.$((i + 1))"
            done
            mv -f "${log_file}" "${log_file}.1"
        fi
    fi
}

# Build a clean per-container runtime workspace that mirrors env/dist while
# keeping INI overrides isolated from host bind mounts.
rm -rf "${RUN_DIR}"
mkdir -p "${RUN_DIR}/config" "${RUN_DIR}/module"

for subdir in systems logs; do
    mkdir -p "${DIST_DIR}/${subdir}"
    ln -s "${DIST_DIR}/${subdir}" "${RUN_DIR}/${subdir}"
done

rotate_log_if_needed "${DIST_DIR}/logs/${SERVER_BIN}.log"

# Support both ./config and ./Config lookups on case-sensitive Linux filesystems
ln -s "${RUN_DIR}/config" "${RUN_DIR}/Config"

# Stage config files: seed *.ini from *.ini.dist if missing, copy *.ini for safe patching,
# and symlink persistent *.dat state files directly to env/dist/config/.
if [[ -d "${DIST_DIR}/config" ]]; then
    for f in "${DIST_DIR}/config"/*.ini.dist; do
        [[ -e "${f}" ]] || continue
        base="$(basename "${f}" .dist)"
        cp "${f}" "${RUN_DIR}/config/${base}.dist"
        if [[ ! -f "${DIST_DIR}/config/${base}" ]]; then
            cp "${f}" "${RUN_DIR}/config/${base}"
        fi
    done
    for f in "${DIST_DIR}/config"/*; do
        [[ -e "${f}" ]] || continue
        base="$(basename "${f}")"
        case "${base}" in
            *.ini.dist)
                ;;
            *.ini|*.INI)
                cp "${f}" "${RUN_DIR}/config/${base}"
                ;;
            *)
                ln -s "${f}" "${RUN_DIR}/config/${base}"
                ;;
        esac
    done
fi

# Stage module configs with .ini.dist fallback layering
if [[ -d "${DIST_DIR}/module" ]]; then
    for f in "${DIST_DIR}/module"/*; do
        [[ -e "${f}" ]] || continue
        base="$(basename "${f}")"
        ln -s "${f}" "${RUN_DIR}/module/${base}"
    done
    for f in "${DIST_DIR}/module"/*.ini.dist; do
        [[ -e "${f}" ]] || continue
        base="$(basename "${f}" .dist)"
        if [[ ! -e "${RUN_DIR}/module/${base}" ]]; then
            cp "${f}" "${RUN_DIR}/module/${base}"
        fi
    done
fi

# Patch runtime copies of DN*.ini cleanly without touching mounted host files:
#   - [Resource] Path= -> GAMERES_PATH
#   - Docker inter-service hostnames when IRINE_DOCKER_NETWORK=1
#   - PublicIp / PrivateIp / GameServer UDP StartPort
for ini in "${RUN_DIR}/config"/DN*.ini; do
    [[ -f "${ini}" ]] || continue
    ini_name="$(basename "${ini}")"
    awk \
        -v ini_name="${ini_name}" \
        -v res_path="${GAMERES_PATH}" \
        -v docker_net="${IRINE_DOCKER_NETWORK:-0}" \
        -v db_host="${IRINE_DB_HOST:-}" \
        -v log_host="${IRINE_LOG_HOST:-irine-logserver}" \
        -v dbsrv_host="${IRINE_DB_SERVER_HOST:-irine-dbserver}" \
        -v cash_host="${IRINE_CASH_HOST:-irine-cashserver}" \
        -v master_host="${IRINE_MASTER_HOST:-irine-masterserver}" \
        -v login_host="${IRINE_LOGIN_HOST:-irine-loginserver}" \
        -v public_ip="${IRINE_PUBLIC_IP:-}" \
        -v private_ip="${IRINE_PRIVATE_IP:-}" \
        -v udp_start="${IRINE_GAME_UDP_START_PORT:-}" \
    '
        BEGIN { sec = "" }
        {
            sub(/\r$/, "", $0)
            if ($0 ~ /^\[[^]]+\][[:space:]]*$/) {
                sec = $0
                sub(/^\[/, "", sec)
                sub(/\][[:space:]]*$/, "", sec)
                print $0
                next
            }
            if (sec == "Resource" && res_path != "" && $0 ~ /^[[:space:]]*Path[[:space:]]*=/) {
                print "Path=" res_path
                next
            }
            if (docker_net == "1") {
                if (db_host != "" && $0 ~ /^[[:space:]]*DBIP[0-9]*[[:space:]]*=[[:space:]]*127\.0\.0\.1([[:space:]]*$)/) {
                    sub(/127\.0\.0\.1/, db_host, $0)
                }
                if (sec == "Connection") {
                    if ($0 ~ /^[[:space:]]*Log[[:space:]]*=[[:space:]]*127\.0\.0\.1:/)    sub(/127\.0\.0\.1/, log_host, $0)
                    if ($0 ~ /^[[:space:]]*DB[0-9]+[[:space:]]*=[[:space:]]*127\.0\.0\.1:/) sub(/127\.0\.0\.1/, dbsrv_host, $0)
                    if ($0 ~ /^[[:space:]]*Cash[[:space:]]*=[[:space:]]*127\.0\.0\.1:/)   sub(/127\.0\.0\.1/, cash_host, $0)
                    if ($0 ~ /^[[:space:]]*Master[0-9]*[[:space:]]*=[[:space:]]*127\.0\.0\.1:/) sub(/127\.0\.0\.1/, master_host, $0)
                    if ($0 ~ /^[[:space:]]*Login[0-9]*[[:space:]]*=[[:space:]]*127\.0\.0\.1:/)  sub(/127\.0\.0\.1/, login_host, $0)
                }
                if (ini_name == "DNGame.ini" && sec == "Info" && udp_start != "" && $0 ~ /^[[:space:]]*StartPort[[:space:]]*=/) {
                    print "StartPort=" udp_start
                    next
                }
            }
            if (public_ip != "" && $0 ~ /^[[:space:]]*PublicIp[[:space:]]*=/) {
                print "PublicIp=" public_ip
                next
            }
            if (private_ip != "" && $0 ~ /^[[:space:]]*PrivateIp[[:space:]]*=/) {
                print "PrivateIp=" private_ip
                next
            }
            print $0
        }
    ' "${ini}" > "${ini}.tmp" && mv "${ini}.tmp" "${ini}"
done

# Warn clearly if a resource-dependent daemon starts with an empty/missing GameRes mount
case "${SERVER_BIN}" in
    DNDBServer64|DNCashServer64|DNMasterServer64|DNLoginServer64|DNVillageServer64|DNGameServer64)
        if [[ ! -d "${GAMERES_PATH}" ]] || [[ -z "$(ls -A "${GAMERES_PATH}" 2>/dev/null)" ]]; then
            echo "[IrineCore][WARN] GameRes directory '${GAMERES_PATH}' is missing or empty." >&2
            echo "[IrineCore][WARN] Set IRINE_GAMERES_PATH in .env or mount GameRes via docker-compose.override.yml." >&2
        else
            echo "[IrineCore] Using GameRes path: ${GAMERES_PATH}"
        fi
        ;;
esac

# Patch runtime odbc.ini with container DB overrides
RUNTIME_ODBC="${RUN_DIR}/config/odbc.ini"
if [[ -f "${RUNTIME_ODBC}" ]]; then
    if [[ -n "${IRINE_DB_HOST:-}" ]]; then
        sed -i -E "s|^Server=.*|Server=${IRINE_DB_HOST}|" "${RUNTIME_ODBC}"
    fi
    if [[ -n "${DN_APP_USER:-}" ]]; then
        sed -i -E "s|^Uid=.*|Uid=${DN_APP_USER}|" "${RUNTIME_ODBC}"
    fi
    if [[ -n "${DN_APP_PASSWORD:-}" ]]; then
        sed -i -E "s|^Pwd=.*|Pwd=${DN_APP_PASSWORD}|" "${RUNTIME_ODBC}"
    fi
    export ODBCINI="${RUNTIME_ODBC}"
fi

cd "${RUN_DIR}"
export LD_LIBRARY_PATH="${DIST_DIR}:${LD_LIBRARY_PATH:-}"

if [[ -x "${DIST_DIR}/${SERVER_BIN}" ]]; then
    echo "[IrineCore] Starting ${SERVER_BIN} (cwd=${RUN_DIR}, dist=${DIST_DIR})..."
    exec "${DIST_DIR}/${SERVER_BIN}"
else
    echo "[IrineCore] Executing custom command: $*"
    exec "$@"
fi
