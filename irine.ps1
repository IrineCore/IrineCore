# =============================================================================
# IrineCore — Unified Cross-Platform Management CLI (irine.ps1)
# =============================================================================
# Single entrypoint for building servers & client, running tests, managing the
# 7-server cluster, Docker Compose, DB migrations, plug-and-play modules,
# log rotation, and preflight diagnostics.
# =============================================================================
[CmdletBinding(PositionalBinding = $false)]
param(
    [Parameter(Position = 0)]
    [string]$Command = "help",
    [Parameter(Position = 1)]
    [string]$SubCommand = "",
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraArgs = @()
)

$ErrorActionPreference = "Stop"
$RootDir = $PSScriptRoot

# Load tracked defaults from conf\dist\config.ps1, then optional local conf\config.ps1
$DistConf  = Join-Path $RootDir "conf\dist\config.ps1"
$LocalConf = Join-Path $RootDir "conf\config.ps1"
if (Test-Path $DistConf)  { . $DistConf }
if (Test-Path $LocalConf) { . $LocalConf }

function Resolve-RepoPath([string]$p, [string]$fallback) {
    $val = if ($p) { $p } else { $fallback }
    if ([System.IO.Path]::IsPathRooted($val)) { return $val }
    return (Join-Path $RootDir $val)
}

$BuildType = if ($IRINE_BUILD_TYPE) { $IRINE_BUILD_TYPE } else { "Release" }
$BuildDir  = Resolve-RepoPath $IRINE_BUILD_DIR "build"
$DistDir   = Resolve-RepoPath $IRINE_DIST_DIR  "env\dist"

function Show-Help {
@"
IrineCore — OpenSource Dragon Nest Server Emulator CLI (Windows)

Usage: .\irine.ps1 <command> [subcommand] [options]

Commands:
  install                          Bootstrap, configure, build, test, and install x64 servers into env\dist
  compiler <configure|build|test|install|all|client|clean>
                                   Run CMake server/client build, CTest, install, or clean
  server <start|stop|restart|status|rotate-logs>
                                   Manage native Windows 7-server cluster and log rotation
  docker <up|down|infra|build|pull|ps|logs|restart>
                                   Manage Docker Compose full cluster or infrastructure-only stack
  db <status|update|dry-run|backup|init> [args...]
                                   Run idempotent core + module SQL migrations and backups
  module <list|enable|disable|add|update|create|validate> [args...]
                                   Manage plug-and-play modules under modules\
  doctor                           Run preflight diagnostics (toolchain, ODBC, GameRes, configs, DB, ports)
  help                             Show this help message

Examples:
  .\irine.ps1 compiler build
  .\irine.ps1 compiler client
  .\irine.ps1 server start -Background
  .\irine.ps1 docker infra
  .\irine.ps1 db status
  .\irine.ps1 module list
  .\irine.ps1 module create My-Feature
  .\irine.ps1 doctor
"@ | Write-Host
}

function Get-PythonBin {
    foreach ($cand in @("python", "python3", "py")) {
        $cmd = Get-Command $cand -ErrorAction SilentlyContinue
        if ($cmd) { return $cmd.Source }
    }
    throw "[IrineCore] Python 3 is required for this command."
}

function Invoke-RotateLogs {
    param([int]$Keep = 5)
    $logDir = Join-Path $DistDir "logs"
    if (-not (Test-Path $logDir)) {
        Write-Host "[IrineCore] No log directory at $logDir."
        return
    }
    $rotated = 0
    foreach ($lf in (Get-ChildItem -Path $logDir -Filter "*.log" -File -ErrorAction SilentlyContinue)) {
        if ($lf.Length -eq 0) { continue }
        for ($i = $Keep - 1; $i -ge 1; $i--) {
            $src = "$($lf.FullName).$i"
            $dst = "$($lf.FullName).$($i + 1)"
            if (Test-Path $src) { Move-Item -Path $src -Destination $dst -Force }
        }
        Copy-Item -Path $lf.FullName -Destination "$($lf.FullName).1" -Force
        Clear-Content -Path $lf.FullName
        Write-Host "[IrineCore] Rotated $($lf.Name) ($($lf.Length) bytes)"
        $rotated++
    }
    Write-Host "[IrineCore] Log rotation complete ($rotated file(s) rotated)."
}

function Invoke-Doctor {
    $script:DoctorFails = 0
    $script:DoctorWarns = 0
    function Ok([string]$m)   { Write-Host "  [OK]   $m" -ForegroundColor Green }
    function Warn([string]$m) { Write-Host "  [WARN] $m" -ForegroundColor Yellow; $script:DoctorWarns++ }
    function Fail([string]$m) { Write-Host "  [FAIL] $m" -ForegroundColor Red; $script:DoctorFails++ }

    Write-Host "=== IrineCore Preflight Diagnostics ===" -ForegroundColor Cyan
    Write-Host "[1/5] Toolchain"
    foreach ($t in @("cmake", "git")) {
        $c = Get-Command $t -ErrorAction SilentlyContinue
        if ($c) { Ok "$t ($($c.Source))" } else { Fail "Missing $t" }
    }
    foreach ($t in @("python", "docker")) {
        $c = Get-Command $t -ErrorAction SilentlyContinue
        if ($c) { Ok "$t available" } else { Warn "Optional tool '$t' not found" }
    }

    Write-Host "[2/5] ODBC Driver"
    try {
        $drivers = Get-OdbcDriver -Platform 64-bit -ErrorAction SilentlyContinue | Where-Object { $_.Name -match "SQL Server" }
        if ($drivers) { Ok ("Found 64-bit SQL ODBC driver(s): " + (($drivers | Select-Object -ExpandProperty Name) -join ", ")) }
        else { Warn "No 64-bit SQL Server ODBC driver detected" }
    } catch {
        Warn "Unable to query 64-bit ODBC drivers"
    }

    Write-Host "[3/5] Staged Distribution (env\dist)"
    if (Test-Path $DistDir) {
        foreach ($ini in @("DNLog.ini","DNDB.ini","DNCash.ini","DNLogin.ini","DNMaster.ini","DNVillage.ini","DNGame.ini","odbc.ini")) {
            $p1 = Join-Path $DistDir "config\$ini"
            $p2 = Join-Path $DistDir "config\$ini.dist"
            if ((Test-Path $p1) -or (Test-Path $p2)) { Ok "config\$ini" }
            else { Warn "Missing config\$ini (run .\irine.ps1 compiler install)" }
        }
        if (Test-Path (Join-Path $DistDir "systems\ChannelInfo.xml")) { Ok "systems\ChannelInfo.xml" } else { Warn "Missing systems\ChannelInfo.xml" }
        if (Test-Path (Join-Path $DistDir "module\modules.manifest")) { Ok "module\modules.manifest" } else { Warn "Missing module\modules.manifest" }
    } else {
        Warn "env\dist not staged yet (run .\irine.ps1 compiler all)"
    }

    Write-Host "[4/5] GameRes Path"
    $gameRes = $env:IRINE_GAMERES_PATH
    if (-not $gameRes -and (Test-Path (Join-Path $RootDir ".env"))) {
        $m = Select-String -Path (Join-Path $RootDir ".env") -Pattern '^IRINE_GAMERES_PATH=(.+)$' | Select-Object -Last 1
        if ($m) { $gameRes = $m.Matches[0].Groups[1].Value.Trim().Trim('"').Trim("'") }
    }
    if (-not $gameRes -and (Test-Path (Join-Path $DistDir "config\DNGame.ini"))) {
        $m = Select-String -Path (Join-Path $DistDir "config\DNGame.ini") -Pattern '^\s*Path\s*=(.+)$' | Select-Object -First 1
        if ($m) { $gameRes = $m.Matches[0].Groups[1].Value.Trim() }
    }
    if ($gameRes -and (Test-Path $gameRes) -and ((Get-ChildItem -Path $gameRes -ErrorAction SilentlyContinue | Measure-Object).Count -gt 0)) {
        Ok "GameRes found and non-empty: $gameRes"
    } else {
        Warn "GameRes directory missing or empty at '$gameRes' (set IRINE_GAMERES_PATH in .env or [Resource] Path= in config\DN*.ini)"
    }

    Write-Host "[5/5] SQL Server Connectivity (127.0.0.1:1433)"
    try {
        $tcp = New-Object System.Net.Sockets.TcpClient
        $ar  = $tcp.BeginConnect("127.0.0.1", 1433, $null, $null)
        if ($ar.AsyncWaitHandle.WaitOne(1200, $false) -and $tcp.Connected) {
            Ok "SQL Server port 1433 is reachable on 127.0.0.1"
        } else {
            Warn "SQL Server port 1433 not reachable on 127.0.0.1 (start via .\irine.ps1 docker infra)"
        }
        $tcp.Close()
    } catch {
        Warn "SQL Server port 1433 not reachable on 127.0.0.1"
    }

    Write-Host "=== Summary: $($script:DoctorFails) error(s), $($script:DoctorWarns) warning(s) ===" -ForegroundColor Cyan
    if ($script:DoctorFails -gt 0) { exit 1 }
}

switch ($Command.ToLowerInvariant()) {
    "install" {
        & (Join-Path $RootDir "apps\installer\install.ps1") @ExtraArgs
    }
    "compiler" {
        $sub = if ($SubCommand) { $SubCommand.ToLowerInvariant() } else { "build" }
        $cmakeArgs = @(
            "-S", $RootDir,
            "-B", $BuildDir,
            "-A", "x64",
            "-DCMAKE_INSTALL_PREFIX=$DistDir",
            "-DIRINE_MODULES=$(if ($IRINE_MODULES) { $IRINE_MODULES } else { 'static' })",
            "-DIRINE_BUILD_TOOLS=$(if ($IRINE_BUILD_TOOLS) { $IRINE_BUILD_TOOLS } else { 'ON' })"
        )
        if ($DISABLED_IRINE_MODULES) { $cmakeArgs += "-DDISABLED_IRINE_MODULES=$DISABLED_IRINE_MODULES" }
        if ($IRINE_CMAKE_EXTRA_ARGS) { $cmakeArgs += $IRINE_CMAKE_EXTRA_ARGS }

        switch ($sub) {
            "configure" { cmake @cmakeArgs @ExtraArgs }
            "build"     {
                if (-not (Test-Path (Join-Path $BuildDir "CMakeCache.txt"))) {
                    cmake @cmakeArgs
                    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
                }
                cmake --build $BuildDir --config $BuildType --parallel @ExtraArgs -- /nr:false
            }
            "test"      { ctest --test-dir $BuildDir -C $BuildType --output-on-failure @ExtraArgs }
            "install"   { cmake --install $BuildDir --config $BuildType @ExtraArgs }
            "all"       { & (Join-Path $RootDir "apps\compiler\compile.ps1") @ExtraArgs }
            "client"    {
                $clientArgs = @()
                if ($IRINE_CLIENT_ARCH) { $clientArgs += @("-Arch", $IRINE_CLIENT_ARCH) }
                if ($IRINE_CLIENT_LTO)  { $clientArgs += "-LTO" }
                & (Join-Path $RootDir "apps\compiler\compile-client.ps1") @clientArgs @ExtraArgs
            }
            "clean"     { if (Test-Path $BuildDir) { Remove-Item $BuildDir -Recurse -Force }; Write-Host "[IrineCore] Cleaned $BuildDir" }
            default     { throw "Unknown compiler subcommand: $SubCommand (valid: configure|build|test|install|all|client|clean)" }
        }
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    "server" {
        $sub = if ($SubCommand) { $SubCommand.ToLowerInvariant() } else { "status" }
        if ($sub -eq "rotate-logs") {
            $keep = if ($IRINE_LOG_ROTATE_KEEP) { [int]$IRINE_LOG_ROTATE_KEEP } else { 5 }
            Invoke-RotateLogs -Keep $keep
        } else {
            & (Join-Path $RootDir "scripts\server.ps1") $sub @ExtraArgs
        }
    }
    "docker" {
        $sub = if ($SubCommand) { $SubCommand.ToLowerInvariant() } else { "ps" }
        Push-Location $RootDir
        try {
            switch ($sub) {
                "up"      { docker compose up -d --build @ExtraArgs }
                "down"    { docker compose down @ExtraArgs }
                "infra"   { & (Join-Path $RootDir "apps\docker\deploy-infra.ps1") @ExtraArgs }
                "build"   { docker compose build @ExtraArgs }
                "pull"    { docker compose pull @ExtraArgs }
                "ps"      { docker compose ps @ExtraArgs }
                "logs"    { docker compose logs -f --tail=100 @ExtraArgs }
                "restart" { docker compose restart @ExtraArgs }
                default   { throw "Unknown docker subcommand: $SubCommand (valid: up|down|infra|build|pull|ps|logs|restart)" }
            }
            if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        } finally {
            Pop-Location
        }
    }
    "db" {
        $py  = Get-PythonBin
        $sub = if ($SubCommand) { $SubCommand.ToLowerInvariant() } else { "status" }
        $script = Join-Path $RootDir "apps\db-assembler\db_assembler.py"
        if ($sub -eq "dry-run") {
            & $py $script update --dry-run @ExtraArgs
        } else {
            & $py $script $sub @ExtraArgs
        }
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    "module" {
        $py     = Get-PythonBin
        $sub    = if ($SubCommand) { $SubCommand.ToLowerInvariant() } else { "list" }
        $script = Join-Path $RootDir "apps\module-manager\module_manager.py"
        & $py $script $sub @ExtraArgs
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    "doctor" {
        Invoke-Doctor
    }
    { $_ -in @("help", "-h", "--help", "-?") } {
        Show-Help
    }
    default {
        Write-Error "Unknown command: $Command"
        Show-Help
        exit 1
    }
}
