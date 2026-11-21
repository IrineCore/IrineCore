# =============================================================================
# IrineCore — deploy/windows/stop.ps1
# Stops all 7 IrineCore servers gracefully in reverse dependency order.
# =============================================================================
param(
    [string]$DistDir = (Join-Path (Resolve-Path "$PSScriptRoot\..\..").Path "env\dist"),
    [int]$GracefulTimeoutSeconds = 5
)

$PidDir = Join-Path $DistDir "run"

$ServersReverse = @(
    "DNGameServer64",
    "DNVillageServer64",
    "DNLoginServer64",
    "DNMasterServer64",
    "DNCashServer64",
    "DNDBServer64",
    "DNLogServer64"
)

foreach ($srv in $ServersReverse) {
    $pidFile = Join-Path $PidDir "$srv.pid"
    $targets = @()

    if (Test-Path $pidFile) {
        $savedPid = (Get-Content $pidFile -ErrorAction SilentlyContinue | Select-Object -First 1)
        if ($savedPid -match '^\d+$') {
            $p = Get-Process -Id ([int]$savedPid) -ErrorAction SilentlyContinue
            if ($p -and $p.ProcessName -eq $srv) {
                $targets += $p
            }
        }
    }

    if ($targets.Count -eq 0) {
        $targets = @(Get-Process -Name $srv -ErrorAction SilentlyContinue)
    }

    foreach ($proc in $targets) {
        Write-Host "[STOP] Stopping $srv (PID $($proc.Id))..." -ForegroundColor Yellow
        try {
            if ($proc.MainWindowHandle -ne [IntPtr]::Zero) {
                $null = $proc.CloseMainWindow()
                $null = $proc.WaitForExit($GracefulTimeoutSeconds * 1000)
            }
            if (-not $proc.HasExited) {
                Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
                $null = $proc.WaitForExit(3000)
            }
        } catch {
            Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
        }
    }

    if (Test-Path $pidFile) {
        Remove-Item $pidFile -Force -ErrorAction SilentlyContinue
    }
    Start-Sleep -Milliseconds 300
}

Write-Host "[IrineCore] All servers stopped." -ForegroundColor Green
