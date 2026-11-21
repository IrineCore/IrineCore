# =============================================================================
# IrineCore — scripts/server.ps1
# Convenience launcher for native Windows server cluster management.
# Usage:
#   .\scripts\server.ps1 start [-Background] [-SkipDbCheck] [-DistDir <path>]
#   .\scripts\server.ps1 stop [-DistDir <path>]
#   .\scripts\server.ps1 restart [-Background] [-SkipDbCheck] [-DistDir <path>]
#   .\scripts\server.ps1 status [-DistDir <path>]
# =============================================================================
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet("start", "stop", "restart", "status")]
    [string]$Action,
    [string]$DistDir = "",
    [int]$StepDelaySeconds = 2,
    [switch]$SkipDbCheck,
    [switch]$Background
)

$ErrorActionPreference = "Stop"
$RepoRoot = (Resolve-Path "$PSScriptRoot\..").Path
$WinDir   = Join-Path $RepoRoot "deploy\windows"
if (-not $DistDir) {
    $DistDir = Join-Path $RepoRoot "env\dist"
}

switch ($Action) {
    "start"   { & (Join-Path $WinDir "start.ps1")   -DistDir $DistDir -StepDelaySeconds $StepDelaySeconds -SkipDbCheck:$SkipDbCheck -Background:$Background }
    "stop"    { & (Join-Path $WinDir "stop.ps1")    -DistDir $DistDir }
    "restart" { & (Join-Path $WinDir "restart.ps1") -DistDir $DistDir -StepDelaySeconds $StepDelaySeconds -SkipDbCheck:$SkipDbCheck -Background:$Background }
    "status"  { & (Join-Path $WinDir "status.ps1")  -DistDir $DistDir }
}
