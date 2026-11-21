# =============================================================================
# IrineCore — deploy/windows/start.ps1
# Starts all 7 IrineCore servers from env/dist in dependency order:
#   DNLogServer64 -> DNDBServer64 -> DNCashServer64 -> DNMasterServer64 ->
#   DNLoginServer64 -> DNVillageServer64 -> DNGameServer64
# =============================================================================
param(
    [string]$DistDir = (Join-Path (Resolve-Path "$PSScriptRoot\..\..").Path "env\dist"),
    [int]$StepDelaySeconds = 2,
    [switch]$SkipDbCheck,
    [switch]$Background
)

$ErrorActionPreference = "Stop"
$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
$PidDir   = Join-Path $DistDir "run"
$LogDir   = Join-Path $DistDir "logs"

if (-not (Test-Path $DistDir)) {
    throw "[IrineCore] Distribution directory not found: $DistDir. Build and install servers first."
}

New-Item -ItemType Directory -Path $PidDir, $LogDir -Force | Out-Null

if (-not $SkipDbCheck) {
    $sqlUp = $false
    try {
        $tcp = New-Object System.Net.Sockets.TcpClient
        $ar = $tcp.BeginConnect("127.0.0.1", 1433, $null, $null)
        if ($ar.AsyncWaitHandle.WaitOne(1500, $false) -and $tcp.Connected) {
            $sqlUp = $true
        }
        $tcp.Close()
    } catch { }

    if (-not $sqlUp) {
        if (Get-Command docker -ErrorAction SilentlyContinue) {
            Write-Host "[IrineCore] SQL Server not detected on 127.0.0.1:1433. Starting irine-database + irine-webserver via Docker..." -ForegroundColor Cyan
            & (Join-Path $RepoRoot "apps\docker\deploy-infra.ps1") -RootDir $RepoRoot
        } else {
            Write-Warning "[IrineCore] SQL Server is not listening on 127.0.0.1:1433 and Docker CLI is not installed. Servers may fail to connect."
        }
    } else {
        Write-Host "[OK] SQL Server reachable on 127.0.0.1:1433." -ForegroundColor Green
    }
}

$Servers = @(
    "DNLogServer64",
    "DNDBServer64",
    "DNCashServer64",
    "DNMasterServer64",
    "DNLoginServer64",
    "DNVillageServer64",
    "DNGameServer64"
)

foreach ($srv in $Servers) {
    $exe = Join-Path $DistDir "$srv.exe"
    if (-not (Test-Path $exe)) {
        throw "[IrineCore] Server executable not found: $exe"
    }

    $existing = Get-Process -Name $srv -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($existing) {
        Set-Content -Path (Join-Path $PidDir "$srv.pid") -Value $existing.Id
        Write-Host "[SKIP] $srv is already running (PID $($existing.Id))." -ForegroundColor Yellow
        continue
    }

    if ($Background) {
        $outLog = Join-Path $LogDir "${srv}_stdout.log"
        $errLog = Join-Path $LogDir "${srv}_stderr.log"
        Write-Host "[START] Launching $srv in background..." -ForegroundColor Cyan
        $proc = Start-Process -FilePath $exe `
            -WorkingDirectory $DistDir `
            -WindowStyle Hidden `
            -RedirectStandardOutput $outLog `
            -RedirectStandardError $errLog `
            -PassThru
    } else {
        Write-Host "[START] Launching $srv ..." -ForegroundColor Cyan
        $proc = Start-Process -FilePath $exe `
            -WorkingDirectory $DistDir `
            -PassThru
    }

    Set-Content -Path (Join-Path $PidDir "$srv.pid") -Value $proc.Id
    Start-Sleep -Seconds $StepDelaySeconds

    if ($proc.HasExited) {
        Write-Error "[IrineCore] $srv exited immediately with code $($proc.ExitCode). Check logs under $LogDir."
        exit 1
    }
    Write-Host "[OK] $srv running (PID $($proc.Id))." -ForegroundColor Green
}

Write-Host "[IrineCore] All 7 servers started." -ForegroundColor Green
