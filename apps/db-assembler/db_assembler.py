#!/usr/bin/env python3
"""
IrineCore — Database Migration & Module SQL Assembler
=====================================================
Cross-platform (Windows & Linux) idempotent SQL migration runner, module SQL
auto-loader, and database backup tool for all 5 IrineCore SQL Server databases
(dngsm, dnmembership, dnserverlog, dnstaging, dnworld).

Tracks applied scripts in `dnmembership.dbo.IrineSchemaMigrations` using
deterministic ordering and SHA-256 checksums.

Usage:
  python3 apps/db-assembler/db_assembler.py status
  python3 apps/db-assembler/db_assembler.py update [--dry-run]
  python3 apps/db-assembler/db_assembler.py backup [--out <dir>]
  python3 apps/db-assembler/db_assembler.py init [--dry-run]
"""

import argparse
import hashlib
import os
import re
import shutil
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parents[2]
DATABASES = ("dngsm", "dnmembership", "dnserverlog", "dnstaging", "dnworld")

ENSURE_MIGRATIONS_TABLE_SQL = """
SET NOCOUNT ON;
IF DB_ID(N'dnmembership') IS NULL
BEGIN
    RAISERROR(N'Database dnmembership does not exist yet. Run init first.', 16, 1);
    RETURN;
END
IF OBJECT_ID(N'dnmembership.dbo.IrineSchemaMigrations', N'U') IS NULL
BEGIN
    CREATE TABLE dnmembership.dbo.IrineSchemaMigrations (
        MigrationKey NVARCHAR(260) NOT NULL PRIMARY KEY,
        SourceName   NVARCHAR(128) NOT NULL,
        TargetDb     NVARCHAR(64)  NOT NULL,
        Sha256       CHAR(64)      NOT NULL,
        AppliedAtUtc DATETIME2(0)  NOT NULL CONSTRAINT DF_IrineSchemaMigrations_AppliedAt DEFAULT SYSUTCDATETIME()
    );
END
"""


def load_dotenv(path: Path) -> dict:
    env = {}
    if not path.is_file():
        return env
    for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        k, v = line.split("=", 1)
        k = k.strip()
        v = v.strip().strip('"').strip("'")
        env[k] = v
    return env


def resolve_connection(args) -> dict:
    dotenv = load_dotenv(ROOT_DIR / ".env")
    mssql_env = load_dotenv(ROOT_DIR / "deploy" / "mssql" / ".env")

    def pick(cli_val, env_key, default):
        if cli_val:
            return cli_val
        if os.environ.get(env_key):
            return os.environ[env_key]
        if dotenv.get(env_key):
            return dotenv[env_key]
        if mssql_env.get(env_key):
            return mssql_env[env_key]
        return default

    port = pick(args.port, "MSSQL_PORT", "1433")
    host = pick(args.server, "IRINE_DB_HOST", "127.0.0.1")
    server = host if ("," in host or "\\" in host) else f"{host},{port}"
    user = pick(args.user, "MSSQL_ADMIN_USER", "sa")
    password = pick(args.password, "MSSQL_SA_PASSWORD", "DnCap60!Local2026")
    container = args.docker_container or os.environ.get("IRINE_MSSQL_CONTAINER", "irine-database")
    return {
        "server": server,
        "user": user,
        "password": password,
        "container": container,
        "use_docker": args.docker,
    }


def detect_sqlcmd_runner(conn: dict) -> list:
    if not conn["use_docker"]:
        for candidate in (
            shutil.which("sqlcmd"),
            "/opt/mssql-tools18/bin/sqlcmd",
            "/opt/mssql-tools/bin/sqlcmd",
        ):
            if candidate and os.path.isfile(candidate):
                return [candidate, "-C", "-S", conn["server"], "-U", conn["user"], "-I", "-b"]

    docker_bin = shutil.which("docker")
    if docker_bin:
        probe = subprocess.run(
            [docker_bin, "ps", "--filter", f"name=^{conn['container']}$", "--format", "{{.Names}}"],
            capture_output=True,
            text=True,
        )
        if probe.returncode == 0 and conn["container"] in probe.stdout.splitlines():
            return [
                docker_bin, "exec", "-i",
                "-e", f"SQLCMDPASSWORD={conn['password']}",
                conn["container"],
                "/opt/mssql-tools18/bin/sqlcmd",
                "-C", "-S", "127.0.0.1", "-U", conn["user"], "-I", "-b",
            ]

    raise RuntimeError(
        "Neither local `sqlcmd` nor a running Docker container "
        f"'{conn['container']}' was found. Install mssql-tools18 or start `irine-database`."
    )


def run_sql_query(runner: list, conn: dict, sql_text: str, database: str = "") -> str:
    cmd = list(runner)
    if database:
        cmd.extend(["-d", database])
    cmd.extend(["-h", "-1", "-W", "-s", "|"])
    env = os.environ.copy()
    env["SQLCMDPASSWORD"] = conn["password"]
    proc = subprocess.run(
        cmd,
        input=sql_text,
        text=True,
        capture_output=True,
        env=env,
    )
    if proc.returncode != 0:
        err = (proc.stderr or proc.stdout).strip()
        raise RuntimeError(f"sqlcmd failed (rc={proc.returncode}): {err}")
    return proc.stdout


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def infer_target_db(path: Path) -> str:
    lower_parts = [p.lower() for p in path.parts]
    for db in DATABASES:
        if db in lower_parts:
            return db
    text = path.read_text(encoding="utf-8", errors="ignore")
    m = re.search(r"(?im)^\s*USE\s+\[?(\w+)\]?", text)
    if m and m.group(1).lower() in DATABASES:
        return m.group(1).lower()
    return "master"


def discover_migrations() -> list:
    """
    Collects all SQL migrations in deterministic order:
      1. Core updates: data/sql/updates/**/*.sql
      2. Enabled module SQL: modules/<Mod>/data/sql/{base,updates}/**/*.sql and modules/<Mod>/data/sql/*.sql
    Modules with a `.disabled` marker file or under `modules/disabled/` are skipped.
    """
    migrations = []

    core_updates = ROOT_DIR / "data" / "sql" / "updates"
    if core_updates.is_dir():
        for sql_file in sorted(core_updates.rglob("*.sql")):
            rel = sql_file.relative_to(ROOT_DIR).as_posix()
            migrations.append({
                "key": rel,
                "source": "core",
                "target_db": infer_target_db(sql_file),
                "path": sql_file,
                "sha256": sha256_file(sql_file),
            })

    modules_dir = ROOT_DIR / "modules"
    if modules_dir.is_dir():
        for mod_dir in sorted(p for p in modules_dir.iterdir() if p.is_dir()):
            name = mod_dir.name
            if name.startswith(".") or name == "disabled" or (mod_dir / ".disabled").exists():
                continue
            sql_root = mod_dir / "data" / "sql"
            if not sql_root.is_dir():
                continue

            # Deterministic phase order inside each module: base -> root -> updates
            ordered_files = []
            seen = set()
            for sub in (sql_root / "base", sql_root, sql_root / "updates"):
                if not sub.is_dir():
                    continue
                iterator = sorted(sub.glob("*.sql")) if sub == sql_root else sorted(sub.rglob("*.sql"))
                for sf in iterator:
                    if sf not in seen:
                        seen.add(sf)
                        ordered_files.append(sf)

            for sql_file in ordered_files:
                rel = sql_file.relative_to(ROOT_DIR).as_posix()
                migrations.append({
                    "key": rel,
                    "source": f"module:{name}",
                    "target_db": infer_target_db(sql_file),
                    "path": sql_file,
                    "sha256": sha256_file(sql_file),
                })

    return migrations


def fetch_applied_migrations(runner: list, conn: dict) -> dict:
    run_sql_query(runner, conn, ENSURE_MIGRATIONS_TABLE_SQL)
    out = run_sql_query(
        runner,
        conn,
        "SET NOCOUNT ON; SELECT MigrationKey, Sha256, CONVERT(VARCHAR(19), AppliedAtUtc, 120) "
        "FROM dnmembership.dbo.IrineSchemaMigrations ORDER BY AppliedAtUtc, MigrationKey;",
    )
    applied = {}
    for line in out.splitlines():
        line = line.strip()
        if not line or "|" not in line:
            continue
        parts = [p.strip() for p in line.split("|")]
        if len(parts) >= 3:
            applied[parts[0]] = {"sha256": parts[1], "applied_at": parts[2]}
    return applied


def cmd_status(args) -> int:
    conn = resolve_connection(args)
    migrations = discover_migrations()
    if args.offline:
        print(f"[IrineCore DB] Discovered {len(migrations)} SQL migration file(s) (offline mode):")
        for m in migrations:
            print(f"  - [{m['source']}] ({m['target_db']}) {m['key']}  sha256={m['sha256'][:12]}")
        return 0

    runner = detect_sqlcmd_runner(conn)
    applied = fetch_applied_migrations(runner, conn)

    pending = 0
    drifted = 0
    print(f"[IrineCore DB] Migration Status ({len(migrations)} discovered, {len(applied)} recorded):")
    for m in migrations:
        rec = applied.get(m["key"])
        if rec is None:
            pending += 1
            print(f"  [PENDING]  ({m['source']} -> {m['target_db']}) {m['key']}")
        elif rec["sha256"].lower() != m["sha256"].lower():
            drifted += 1
            print(f"  [MODIFIED] ({m['source']} -> {m['target_db']}) {m['key']} (applied {rec['applied_at']} UTC)")
        else:
            print(f"  [APPLIED]  ({m['source']} -> {m['target_db']}) {m['key']} ({rec['applied_at']} UTC)")

    print(f"[IrineCore DB] Summary: {pending} pending, {drifted} checksum-modified.")
    return 0


def cmd_update(args) -> int:
    conn = resolve_connection(args)
    migrations = discover_migrations()

    if args.dry_run and args.offline:
        print(f"[IrineCore DB] Dry-run (offline): {len(migrations)} migration file(s) discovered.")
        for m in migrations:
            print(f"  -> would apply [{m['source']}] ({m['target_db']}) {m['key']}")
        return 0

    runner = detect_sqlcmd_runner(conn)
    applied = fetch_applied_migrations(runner, conn)

    pending_list = []
    for m in migrations:
        rec = applied.get(m["key"])
        if rec is None:
            pending_list.append(m)
        elif rec["sha256"].lower() != m["sha256"].lower():
            print(
                f"[IrineCore DB][WARN] Checksum changed after apply: {m['key']} "
                f"(recorded={rec['sha256'][:12]}, current={m['sha256'][:12]}). Skipping unless manually reset.",
                file=sys.stderr,
            )

    if not pending_list:
        print("[IrineCore DB] Database is up to date (0 pending migrations).")
        return 0

    if args.dry_run:
        print(f"[IrineCore DB] Dry-run: {len(pending_list)} pending migration(s):")
        for m in pending_list:
            print(f"  -> would apply [{m['source']}] ({m['target_db']}) {m['key']}")
        return 0

    for m in pending_list:
        print(f"[IrineCore DB] Applying [{m['source']}] ({m['target_db']}) {m['key']} ...")
        sql_body = m["path"].read_text(encoding="utf-8", errors="replace").replace("\r\n", "\n")
        target_db = m["target_db"] if m["target_db"] != "master" else ""
        run_sql_query(runner, conn, sql_body, database=target_db)

        safe_key = m["key"].replace("'", "''")
        safe_src = m["source"].replace("'", "''")
        safe_db = m["target_db"].replace("'", "''")
        record_sql = (
            "SET NOCOUNT ON; "
            "INSERT INTO dnmembership.dbo.IrineSchemaMigrations (MigrationKey, SourceName, TargetDb, Sha256) "
            f"VALUES (N'{safe_key}', N'{safe_src}', N'{safe_db}', '{m['sha256']}');"
        )
        run_sql_query(runner, conn, record_sql)

    print(f"[IrineCore DB] Applied {len(pending_list)} migration(s) successfully.")
    return 0


def cmd_backup(args) -> int:
    conn = resolve_connection(args)
    runner = detect_sqlcmd_runner(conn)
    ts = datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S")
    out_dir = args.out or "/var/opt/mssql/backup"

    print(f"[IrineCore DB] Backing up all 5 databases to '{out_dir}' (timestamp {ts} UTC)...")
    for db in DATABASES:
        bak_path = f"{out_dir.rstrip('/')}/{db}_{ts}.bak"
        safe_bak = bak_path.replace("'", "''")
        sql = (
            f"BACKUP DATABASE [{db}] TO DISK = N'{safe_bak}' "
            f"WITH INIT, COMPRESSION, STATS = 25, NAME = N'IrineCore-{db}-{ts}';"
        )
        run_sql_query(runner, conn, sql)
        print(f"  [OK] {db} -> {bak_path}")
    print("[IrineCore DB] Backup complete.")
    return 0


def cmd_init(args) -> int:
    if os.name == "nt" and not args.docker:
        ps1 = ROOT_DIR / "deploy" / "mssql" / "scripts" / "populate-db.ps1"
        conn = resolve_connection(args)
        cmd = [
            "powershell", "-ExecutionPolicy", "Bypass", "-File", str(ps1),
            "-Server", conn["server"],
            "-SaPassword", conn["password"],
        ]
        if args.dry_run:
            print("[IrineCore DB] Dry-run would execute:", " ".join(cmd))
            return 0
        rc = subprocess.call(cmd)
        if rc != 0:
            return rc
    else:
        print("[IrineCore DB] Ensuring SQL Server container is running and initialized...")
        if not args.dry_run:
            rc = subprocess.call(
                ["docker", "compose", "up", "-d", "--build", "--wait", "irine-database"],
                cwd=str(ROOT_DIR),
            )
            if rc != 0:
                return rc
    return cmd_update(args)


def main() -> int:
    parser = argparse.ArgumentParser(
        prog="db_assembler.py",
        description="IrineCore Database Migration & Module SQL Assembler",
    )
    parser.add_argument("--server", help="SQL Server host or host,port")
    parser.add_argument("--port", help="SQL Server TCP port (default: 1433)")
    parser.add_argument("--user", help="SQL admin user (default: sa)")
    parser.add_argument("--password", help="SQL admin password (defaults to MSSQL_SA_PASSWORD in .env)")
    parser.add_argument("--docker", action="store_true", help="Force executing sqlcmd inside Docker container")
    parser.add_argument("--docker-container", help="Docker SQL Server container name (default: irine-database)")

    sub = parser.add_subparsers(dest="command", required=True)

    p_status = sub.add_parser("status", help="Show applied and pending SQL migrations")
    p_status.add_argument("--offline", action="store_true", help="List discovered SQL files without connecting to SQL Server")
    p_status.set_defaults(func=cmd_status)

    p_update = sub.add_parser("update", help="Apply all pending core and enabled-module SQL migrations")
    p_update.add_argument("--dry-run", action="store_true", help="Preview pending migrations without modifying the database")
    p_update.add_argument("--offline", action="store_true", help="Dry-run discovery without connecting to SQL Server")
    p_update.set_defaults(func=cmd_update)

    p_backup = sub.add_parser("backup", help="Create compressed .bak backups of all 5 IrineCore databases")
    p_backup.add_argument("--out", help="SQL Server backup directory (default: /var/opt/mssql/backup)")
    p_backup.set_defaults(func=cmd_backup)

    p_init = sub.add_parser("init", help="Initialize base databases and apply pending migrations")
    p_init.add_argument("--dry-run", action="store_true", help="Preview initialization steps")
    p_init.add_argument("--offline", action="store_true", help="Offline dry-run")
    p_init.set_defaults(func=cmd_init)

    args = parser.parse_args()
    try:
        return args.func(args)
    except Exception as exc:
        print(f"[IrineCore DB][ERROR] {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
