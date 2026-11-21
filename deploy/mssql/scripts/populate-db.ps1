# =============================================================================
# IrineCore — deploy/mssql/scripts/populate-db.ps1
# Auto-populates all 5 Dragon Nest databases directly from data/sql/base
# (+ data/sql/updates) against either a running SQL Server Docker container
# or a local/remote SQL Server instance.
# =============================================================================
param(
    [string]$Server = "127.0.0.1,1433",
    [string]$SaPassword = "",
    [string]$AppUser = "",
    [string]$AppPassword = "",
    [string]$ContainerName = "",
    [switch]$ForceFullSchema
)

$ErrorActionPreference = "Stop"

$ScriptDir  = $PSScriptRoot
$MssqlDir   = (Resolve-Path (Join-Path $ScriptDir "..")).Path
$RepoRoot   = (Resolve-Path (Join-Path $MssqlDir "..\..")).Path
$BaseSqlDir = Join-Path $RepoRoot "data\sql\base"
$UpdatesDir = Join-Path $RepoRoot "data\sql\updates"
$EnvFile    = Join-Path $MssqlDir ".env"

if (-not (Test-Path $BaseSqlDir)) {
    throw "[irine-mssql] Base SQL source directory not found: $BaseSqlDir"
}

# Load defaults from deploy/mssql/.env (or .env.example) if parameters not explicitly provided
$EnvSource = if (Test-Path $EnvFile) { $EnvFile } else { Join-Path $MssqlDir ".env.example" }
$EnvMap = @{}
if (Test-Path $EnvSource) {
    Get-Content $EnvSource | ForEach-Object {
        if ($_ -match '^\s*([^#=]+?)\s*=\s*(.*)\s*$') {
            $EnvMap[$matches[1].Trim()] = $matches[2].Trim()
        }
    }
}
if (-not $SaPassword)  { $SaPassword  = if ($EnvMap["MSSQL_SA_PASSWORD"]) { $EnvMap["MSSQL_SA_PASSWORD"] } else { "DnCap60!Local2026" } }
if (-not $AppUser)     { $AppUser     = if ($EnvMap["DN_APP_USER"])       { $EnvMap["DN_APP_USER"] }       else { "DragonNest" } }
if (-not $AppPassword) { $AppPassword = if ($EnvMap["DN_APP_PASSWORD"])   { $EnvMap["DN_APP_PASSWORD"] }   else { "DnCap60!App2026" } }

# Detect execution mode: local sqlcmd vs docker exec
$SqlCmdExe = Get-Command sqlcmd -ErrorAction SilentlyContinue
$UseDockerExec = $false
if ($ContainerName) {
    $UseDockerExec = $true
} elseif (-not $SqlCmdExe) {
    $Detected = docker ps --format '{{.Names}}' 2>$null | Where-Object { $_ -match 'irine-database|mssql' } | Select-Object -First 1
    if ($Detected) {
        $ContainerName = $Detected
        $UseDockerExec = $true
        Write-Host "[irine-mssql] Using sqlcmd inside running Docker container '$ContainerName'..." -ForegroundColor Cyan
    } else {
        throw "[irine-mssql] Neither host 'sqlcmd' nor a running MSSQL Docker container was found. Start Docker MSSQL with 'apps\docker\deploy-infra.ps1' first."
    }
}

# Build ephemeral SQL phases (01..09) into TEMP directly from data/sql/base
$TempSqlDir = Join-Path ([System.IO.Path]::GetTempPath()) ("irine-sql-" + [Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $TempSqlDir -Force | Out-Null

try {
    Write-Host "[irine-mssql] Compiling SQL phases from data/sql/base..." -ForegroundColor Cyan
    $env:SRC_DB_ROOT = $BaseSqlDir
    $env:OUT_SQL_DIR = $TempSqlDir
    if (Get-Command python -ErrorAction SilentlyContinue) {
        python (Join-Path $ScriptDir "build-sql-bundle.py")
    } elseif (Get-Command docker -ErrorAction SilentlyContinue) {
        docker run --rm `
            -e SRC_DB_ROOT=/src/base `
            -e OUT_SQL_DIR=/src/out `
            -v "${BaseSqlDir}:/src/base:ro" `
            -v "${ScriptDir}:/src/scripts:ro" `
            -v "${TempSqlDir}:/src/out" `
            python:3.12-alpine python /src/scripts/build-sql-bundle.py
    } else {
        throw "[irine-mssql] Neither Python nor Docker CLI was found in PATH."
    }
    if ($LASTEXITCODE -ne 0) { throw "Failed to compile SQL bundle from data/sql/base." }

    function Invoke-SqlQuery([string]$Query) {
        if ($UseDockerExec) {
            $res = docker exec -e "SQLCMDPASSWORD=$SaPassword" $ContainerName /opt/mssql-tools18/bin/sqlcmd -C -S 127.0.0.1 -U sa -I -b -h -1 -W -Q $Query
            if ($LASTEXITCODE -ne 0) { throw "SQL query failed inside container $ContainerName" }
            return ($res -join "").Trim()
        } else {
            $env:SQLCMDPASSWORD = $SaPassword
            $res = & $SqlCmdExe.Source -C -S $Server -U sa -I -b -h -1 -W -Q $Query
            if ($LASTEXITCODE -ne 0) { throw "SQL query failed against $Server" }
            return ($res -join "").Trim()
        }
    }

    function Invoke-SqlFile([string]$FilePath, [hashtable]$Vars = @{}) {
        $name = Split-Path $FilePath -Leaf
        Write-Host "[irine-mssql] Running $name ..." -ForegroundColor Cyan
        if ($UseDockerExec) {
            $remotePath = "/tmp/irine_$name"
            docker cp $FilePath "${ContainerName}:${remotePath}" | Out-Null
            $argsList = @("exec", "-e", "SQLCMDPASSWORD=$SaPassword", $ContainerName, "/opt/mssql-tools18/bin/sqlcmd", "-C", "-S", "127.0.0.1", "-U", "sa", "-I", "-b")
            foreach ($kv in $Vars.GetEnumerator()) {
                $argsList += "-v"
                $argsList += "$($kv.Key)=$($kv.Value)"
            }
            $argsList += @("-i", $remotePath)
            & docker @argsList
            $code = $LASTEXITCODE
            docker exec $ContainerName rm -f $remotePath 2>$null | Out-Null
            if ($code -ne 0) { throw "Failed executing $name" }
        } else {
            $env:SQLCMDPASSWORD = $SaPassword
            $argsList = @("-C", "-S", $Server, "-U", "sa", "-I", "-b")
            foreach ($kv in $Vars.GetEnumerator()) {
                $argsList += "-v"
                $argsList += "$($kv.Key)=$($kv.Value)"
            }
            $argsList += @("-i", $FilePath)
            & $SqlCmdExe.Source @argsList
            if ($LASTEXITCODE -ne 0) { throw "Failed executing $name" }
        }
    }

    $dbCount = Invoke-SqlQuery "SET NOCOUNT ON; SELECT COUNT(*) FROM sys.databases WHERE name IN (N'dngsm',N'dnmembership',N'dnserverlog',N'dnstaging',N'dnworld');"
    $coreReady = "0"
    if ($dbCount -eq "5") {
        $coreReady = Invoke-SqlQuery "SET NOCOUNT ON; SELECT CASE WHEN OBJECT_ID(N'dnworld.dbo.Characters',N'U') IS NOT NULL AND OBJECT_ID(N'dnmembership.dbo.Accounts',N'U') IS NOT NULL AND OBJECT_ID(N'dnworld.dbo.P_AddDWCCharacter',N'P') IS NOT NULL THEN 1 ELSE 0 END;"
    }

    if ($ForceFullSchema -or $dbCount -eq "0" -or ($coreReady -ne "1")) {
        if (-not $ForceFullSchema -and $dbCount -ne "0") {
            $existingTables = Invoke-SqlQuery "SET NOCOUNT ON; DECLARE @c int = 0; IF DB_ID(N'dnworld') IS NOT NULL SELECT @c += COUNT(*) FROM dnworld.sys.tables; IF DB_ID(N'dnmembership') IS NOT NULL SELECT @c += COUNT(*) FROM dnmembership.sys.tables; SELECT @c;"
            if ($existingTables -ne "0") {
                throw "[irine-mssql] Partial populated DB detected ($dbCount/5 DBs, $existingTables tables). Re-run with -ForceFullSchema only if intentional."
            }
        }
        foreach ($phase in @("01_databases.sql","02_principals.sql","03_tables.sql","04_foreign_keys.sql","05_functions.sql","06_views.sql","07_triggers.sql","08_procedures.sql")) {
            Invoke-SqlFile (Join-Path $TempSqlDir $phase)
        }
    } else {
        Write-Host "[irine-mssql] All 5 databases & core schema already exist; applying updates & post-setup only." -ForegroundColor Yellow
    }

    if (Test-Path $UpdatesDir) {
        Get-ChildItem -Path $UpdatesDir -Filter "*.sql" | Sort-Object Name | ForEach-Object {
            Invoke-SqlFile $_.FullName
        }
    }

    $safeUser = $AppUser -replace "'", "''"
    $safePass = $AppPassword -replace "'", "''"
    Invoke-SqlFile (Join-Path $TempSqlDir "09_post_setup.sql") @{ DN_APP_USER = $safeUser; DN_APP_PASSWORD = $safePass }

    Write-Host "[irine-mssql] All 5 IrineCore databases are populated and ready." -ForegroundColor Green
}
finally {
    if (Test-Path $TempSqlDir) {
        Remove-Item $TempSqlDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}
