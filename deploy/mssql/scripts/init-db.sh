#!/bin/bash
set -euo pipefail
set +H

sql_pid="${1:-}"
SQL_DIR="/opt/dn-init/sql"
UPDATES_DIR="/opt/dn-init/updates"
INIT_MARKER="/var/opt/mssql/.dn_initialized"

# Remove marker during boot validation/initialization so Docker healthcheck
# never reports healthy before all SQL phases and updates finish.
rm -f "$INIT_MARKER"

SQLCMD="/opt/mssql-tools18/bin/sqlcmd"
if [ ! -x "$SQLCMD" ]; then
    SQLCMD="/opt/mssql-tools/bin/sqlcmd"
fi
if [ ! -x "$SQLCMD" ]; then
    echo "[dn-mssql] ERROR: sqlcmd binary not found." >&2
    exit 1
fi

SA_PW="${MSSQL_SA_PASSWORD:-${SA_PASSWORD:-}}"
if [ -z "$SA_PW" ]; then
    echo "[dn-mssql] ERROR: MSSQL_SA_PASSWORD (or SA_PASSWORD) must be set." >&2
    exit 1
fi

APP_USER="${DN_APP_USER:-DragonNest}"
APP_PW="${DN_APP_PASSWORD:-}"

sqlcmd_sa() {
    SQLCMDPASSWORD="$SA_PW" "$SQLCMD" -C -S 127.0.0.1 -U sa -I -b "$@"
}

echo "[dn-mssql] Waiting for SQL Server to become ready..."
sleep 4
ready=0
for _ in $(seq 1 60); do
    if [ -n "$sql_pid" ] && ! kill -0 "$sql_pid" 2>/dev/null; then
        echo "[dn-mssql] ERROR: sqlservr exited during startup." >&2
        exit 1
    fi
    state_sum="$(sqlcmd_sa -h -1 -W -Q "SET NOCOUNT ON; SELECT ISNULL(SUM(state), 1) FROM sys.databases;" 2>/dev/null | tr -d '[:space:]' || true)"
    if [ "$state_sum" = "0" ]; then
        ready=1
        break
    fi
    sleep 2
done

if [ "$ready" -ne 1 ]; then
    echo "[dn-mssql] ERROR: Timed out waiting for SQL Server readiness." >&2
    exit 1
fi
echo "[dn-mssql] SQL Server is ready."

db_count="$(sqlcmd_sa -h -1 -W -Q "SET NOCOUNT ON; SELECT COUNT(*) FROM sys.databases WHERE name IN (N'dngsm',N'dnmembership',N'dnserverlog',N'dnstaging',N'dnworld');" | tr -d '[:space:]')"

# Count core schema tables across any existing DN databases so we can distinguish
# a clean/partial failed init from a populated live database.
core_schema_ready="0"
if [ "$db_count" = "5" ]; then
    core_schema_ready="$(sqlcmd_sa -h -1 -W -Q "SET NOCOUNT ON; SELECT CASE WHEN OBJECT_ID(N'dnworld.dbo.Characters',N'U') IS NOT NULL AND OBJECT_ID(N'dnmembership.dbo.Accounts',N'U') IS NOT NULL AND OBJECT_ID(N'dnworld.dbo.P_AddDWCCharacter',N'P') IS NOT NULL THEN 1 ELSE 0 END;" | tr -d '[:space:]')"
fi

if [ "$db_count" = "0" ]; then
    echo "[dn-mssql] Fresh SQL Server detected. Auto-populating all 5 IrineCore databases from $SQL_DIR..."
    for phase in \
        01_databases.sql \
        02_principals.sql \
        03_tables.sql \
        04_foreign_keys.sql \
        05_functions.sql \
        06_views.sql \
        07_triggers.sql \
        08_procedures.sql
    do
        echo "[dn-mssql] Running $phase ..."
        sqlcmd_sa -i "$SQL_DIR/$phase"
    done
    echo "[dn-mssql] Base schema & seed data initialized successfully."
elif [ "$db_count" = "5" ] && [ "$core_schema_ready" = "1" ]; then
    echo "[dn-mssql] All 5 databases and core schema already exist; skipping base table/procedure creation."
else
    # Partial database creation (e.g., interrupted first boot): check if any user tables exist
    existing_tables="$(sqlcmd_sa -h -1 -W -Q "SET NOCOUNT ON; DECLARE @cnt int = 0; IF DB_ID(N'dnworld') IS NOT NULL SELECT @cnt += COUNT(*) FROM dnworld.sys.tables; IF DB_ID(N'dnmembership') IS NOT NULL SELECT @cnt += COUNT(*) FROM dnmembership.sys.tables; SELECT @cnt;" | tr -d '[:space:]')"
    if [ "$existing_tables" = "0" ]; then
        echo "[dn-mssql] Empty/interrupted initial database state detected ($db_count/5 DBs, 0 tables). Resuming full auto-population..."
        for phase in \
            01_databases.sql \
            02_principals.sql \
            03_tables.sql \
            04_foreign_keys.sql \
            05_functions.sql \
            06_views.sql \
            07_triggers.sql \
            08_procedures.sql
        do
            echo "[dn-mssql] Running $phase ..."
            sqlcmd_sa -i "$SQL_DIR/$phase"
        done
        echo "[dn-mssql] Resumed auto-population completed successfully."
    else
        echo "[dn-mssql] ERROR: Partial populated database state detected ($db_count/5 DBs, $existing_tables tables). Refusing to overwrite existing data." >&2
        exit 1
    fi
fi

# Ensure idempotent migration ledger table exists in dnmembership
sqlcmd_sa -Q "SET NOCOUNT ON;
IF DB_ID(N'dnmembership') IS NOT NULL AND OBJECT_ID(N'dnmembership.dbo.IrineSchemaMigrations', N'U') IS NULL
BEGIN
    CREATE TABLE dnmembership.dbo.IrineSchemaMigrations (
        MigrationKey NVARCHAR(260) NOT NULL PRIMARY KEY,
        SourceName   NVARCHAR(128) NOT NULL,
        TargetDb     NVARCHAR(64)  NOT NULL,
        Sha256       CHAR(64)      NOT NULL,
        AppliedAtUtc DATETIME2(0)  NOT NULL CONSTRAINT DF_IrineSchemaMigrations_AppliedAt DEFAULT SYSUTCDATETIME()
    );
END"

apply_tracked_migration() {
    local mig_key="$1"
    local mig_src="$2"
    local sql_file="$3"
    local safe_key="${mig_key//\'/\'\'}"
    local safe_src="${mig_src//\'/\'\'}"
    local file_sha
    file_sha="$(sha256sum "$sql_file" | awk '{print $1}')"

    local already
    already="$(sqlcmd_sa -h -1 -W -Q "SET NOCOUNT ON; SELECT COUNT(*) FROM dnmembership.dbo.IrineSchemaMigrations WHERE MigrationKey = N'${safe_key}';" | tr -d '[:space:]')"
    if [ "$already" = "0" ]; then
        echo "[dn-mssql] Applying SQL migration [${mig_src}] ${mig_key} ..."
        sqlcmd_sa -i "$sql_file"
        sqlcmd_sa -Q "SET NOCOUNT ON; INSERT INTO dnmembership.dbo.IrineSchemaMigrations (MigrationKey, SourceName, TargetDb, Sha256) VALUES (N'${safe_key}', N'${safe_src}', N'master', '${file_sha}');"
    else
        echo "[dn-mssql] Skipping already-applied migration ${mig_key}."
    fi
}

# Apply tracked core SQL updates mounted or baked in /opt/dn-init/updates/**/*.sql
if [ -d "$UPDATES_DIR" ]; then
    while IFS= read -r uf; do
        [ -n "$uf" ] || continue
        rel="${uf#${UPDATES_DIR}/}"
        apply_tracked_migration "data/sql/updates/${rel}" "core" "$uf"
    done < <(find "$UPDATES_DIR" -type f -name '*.sql' | sort)
fi

# Auto-discover and apply tracked SQL from enabled modules mounted in /opt/dn-init/modules
MODULES_DIR="/opt/dn-init/modules"
if [ -d "$MODULES_DIR" ]; then
    for mod_dir in "$MODULES_DIR"/*/; do
        [ -d "$mod_dir" ] || continue
        mod_name="$(basename "$mod_dir")"
        case "$mod_name" in
            .*|disabled) continue ;;
        esac
        [ -f "${mod_dir}.disabled" ] && continue
        mod_sql_root="${mod_dir}data/sql"
        [ -d "$mod_sql_root" ] || continue
        for phase_dir in "${mod_sql_root}/base" "${mod_sql_root}" "${mod_sql_root}/updates"; do
            [ -d "$phase_dir" ] || continue
            if [ "$phase_dir" = "$mod_sql_root" ]; then
                find_cmd=(find "$phase_dir" -maxdepth 1 -type f -name '*.sql')
            else
                find_cmd=(find "$phase_dir" -type f -name '*.sql')
            fi
            while IFS= read -r msf; do
                [ -n "$msf" ] || continue
                rel="${msf#${MODULES_DIR}/}"
                apply_tracked_migration "modules/${rel}" "module:${mod_name}" "$msf"
            done < <("${find_cmd[@]}" | sort)
        done
    done
fi

# Always apply idempotent application login, role mappings, and critical seed checks
POST_SETUP_SQL="$SQL_DIR/09_post_setup.sql"

if [ -n "$APP_PW" ] && [ -f "$POST_SETUP_SQL" ]; then
    safe_app_user="${APP_USER//\'/\'\'}"
    safe_app_pw="${APP_PW//\'/\'\'}"
    echo "[dn-mssql] Ensuring SQL login '$APP_USER', role mappings, and seed integrity..."
    sqlcmd_sa -v DN_APP_USER="$safe_app_user" DN_APP_PASSWORD="$safe_app_pw" -i "$POST_SETUP_SQL"
else
    echo "[dn-mssql] DN_APP_PASSWORD is empty; skipping application login setup."
fi

# Final verification before marking initialized (unlocks Docker healthcheck)
verify_ok="$(sqlcmd_sa -h -1 -W -Q "SET NOCOUNT ON;
SELECT CASE
    WHEN (SELECT COUNT(*) FROM sys.databases WHERE state = 0 AND name IN (N'dngsm',N'dnmembership',N'dnserverlog',N'dnstaging',N'dnworld')) = 5
     AND OBJECT_ID(N'dnworld.dbo.P_AddDWCCharacter', N'P') IS NOT NULL
     AND OBJECT_ID(N'dnworld.dbo.P_UpsEquipment', N'P') IS NOT NULL
     AND OBJECT_ID(N'dnmembership.dbo.Auth_ResetAuthByAccountWorld', N'P') IS NOT NULL
     AND OBJECT_ID(N'dnserverlog.dbo.GameLog', N'P') IS NOT NULL
     AND EXISTS (SELECT TOP (1) 1 FROM dnmembership.dbo.DatabaseVersion)
     AND EXISTS (SELECT TOP (1) 1 FROM dnworld.dbo.WorldCoverage)
    THEN 1 ELSE 0 END;" | tr -d '[:space:]')"

if [ "$verify_ok" != "1" ]; then
    echo "[dn-mssql] ERROR: Post-initialization verification failed!" >&2
    exit 1
fi

touch "$INIT_MARKER"
echo "[dn-mssql] Database setup complete and verified."
