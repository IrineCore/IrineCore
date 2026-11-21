# =============================================================================
# IrineCore — apps/compiler/compile.ps1
# Configures, builds, runs CTest, and installs IrineCore into env/dist.
# =============================================================================
param(
    [string]$RootDir   = (Resolve-Path "$PSScriptRoot\..\..").Path,
    [string]$BuildType = "Release"
)

$ErrorActionPreference = "Stop"
$BuildDir = Join-Path $RootDir "build"
$DistDir  = Join-Path $RootDir "env\dist"

Write-Host "[IrineCore] Configuring CMake project..." -ForegroundColor Cyan
cmake -S "$RootDir" -B "$BuildDir" -A x64 -DCMAKE_INSTALL_PREFIX="$DistDir"

Write-Host "[IrineCore] Compiling all libraries, modules, and server applications ($BuildType)..." -ForegroundColor Cyan
cmake --build "$BuildDir" --config $BuildType --parallel

Write-Host "[IrineCore] Running automated test suite..." -ForegroundColor Cyan
ctest --test-dir "$BuildDir" -C $BuildType --output-on-failure

Write-Host "[IrineCore] Installing binaries and configurations into $DistDir ..." -ForegroundColor Green
cmake --install "$BuildDir" --config $BuildType
