# =============================================================================
# IrineCore — deploy/windows/install.ps1
# Builds IrineCore (Release x64) via CMake and stages binaries + configs.
# =============================================================================
param(
    [string]$SourceDir = (Resolve-Path "$PSScriptRoot\..\..").Path,
    [string]$BuildDir  = "$SourceDir\build",
    [string]$DeployDir = "$SourceDir\env\dist"
)

$ErrorActionPreference = "Stop"
Write-Host "[IrineCore] Configuring CMake x64 project at $SourceDir ..." -ForegroundColor Cyan
cmake -S "$SourceDir" -B "$BuildDir" -A x64 -DCMAKE_INSTALL_PREFIX="$DeployDir"

Write-Host "[IrineCore] Building all 7 IrineCore servers & tools (Release x64) ..." -ForegroundColor Cyan
cmake --build "$BuildDir" --config Release --parallel

Write-Host "[IrineCore] Installing servers, tools, configs, and modules to $DeployDir ..." -ForegroundColor Green
cmake --install "$BuildDir" --config Release

Write-Host "[IrineCore] Windows installation & build complete." -ForegroundColor Green
