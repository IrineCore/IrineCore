# =============================================================================
# IrineCore — deploy/windows/status.ps1
# Shows runtime status for all 7 IrineCore servers and SQL Server connectivity.
# =============================================================================
param(
    [string]$DistDir = (Join-Path (Resolve-Path "$PSScriptRoot\..\..").Path "env\dist")
)

$sqlStatus = "DOWN"
try {
    $tcp = New-Object System.Net.Sockets.TcpClient
    $ar = $tcp.BeginConnect("127.0.0.1", 1433, $null, $null)
    if ($ar.AsyncWaitHandle.WaitOne(1000, $false) -and $tcp.Connected) {
        $sqlStatus = "UP (127.0.0.1:1433)"
    }
    $tcp.Close()
} catch { }

Write-Host "SQL Server : $sqlStatus" -ForegroundColor $(if ($sqlStatus -like "UP*") { "Green" } else { "Red" })
Write-Host "Dist Dir   : $DistDir"
Write-Host ""

$Servers = @(
    "DNLogServer64",
    "DNDBServer64",
    "DNCashServer64",
    "DNMasterServer64",
    "DNLoginServer64",
    "DNVillageServer64",
    "DNGameServer64"
)

$rows = foreach ($srv in $Servers) {
    $proc = Get-Process -Name $srv -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($proc) {
        $uptime = (Get-Date) - $proc.StartTime
        [PSCustomObject]@{
            Server   = $srv
            State    = "RUNNING"
            PID      = $proc.Id
            MemoryMB = [math]::Round($proc.WorkingSet64 / 1MB, 1)
            Uptime   = ("{0:dd\.hh\:mm\:ss}" -f $uptime)
        }
    } else {
        [PSCustomObject]@{
            Server   = $srv
            State    = "STOPPED"
            PID      = "-"
            MemoryMB = "-"
            Uptime   = "-"
        }
    }
}

$rows | Format-Table -AutoSize
