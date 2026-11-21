# =============================================================================
# IrineCore — deploy/windows/install.ps1
# Builds IrineCore (Release x64) via CMake and stages binaries + configs.
# =============================================================================
param(
    [string]$SourceDir = (Resolve-Path "$PSScriptRoot\..\..").Path,
    [string]$BuildDir  = "$SourceDir\build",
    [string]$DeployDir = "D:\Cap60\Server"
)

$ErrorActionPreference = "Stop"
Write-Host "[IrineCore] Configuring CMake x64 project at $SourceDir ..." -ForegroundColor Cyan
cmake -S "$SourceDir" -B "$BuildDir" -A x64

Write-Host "[IrineCore] Building all 7 IrineCore server binaries (Release x64) ..." -ForegroundColor Cyan
cmake --build "$BuildDir" --config Release --parallel

if (Test-Path "$DeployDir") {
    Write-Host "[IrineCore] Deploying built server binaries to $DeployDir ..." -ForegroundColor Green
    Copy-Item "$BuildDir\bin\Release\DN*Server64.exe" -Destination "$DeployDir\" -Force
}

Write-Host "[IrineCore] Windows installation & build complete." -ForegroundColor Green
