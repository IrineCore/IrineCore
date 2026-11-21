# =============================================================================
# IrineCore — deploy/windows/restart.ps1
# Restarts the IrineCore server cluster cleanly.
# =============================================================================
param(
    [string]$DistDir = (Join-Path (Resolve-Path "$PSScriptRoot\..\..").Path "env\dist"),
    [int]$StepDelaySeconds = 2,
    [switch]$SkipDbCheck,
    [switch]$Background
)

& "$PSScriptRoot\stop.ps1" -DistDir $DistDir
Start-Sleep -Seconds 2
& "$PSScriptRoot\start.ps1" -DistDir $DistDir -StepDelaySeconds $StepDelaySeconds -SkipDbCheck:$SkipDbCheck -Background:$Background
