# =============================================================================
# IrineCore — apps/compiler/compile-client.ps1
# Configures, builds, and installs Windows DnLauncher & DragonNest client
# binaries into env/client (defaults to native x64; supports -Arch Win32).
# Requires: Visual Studio (C++ & Static MFC toolset) + DirectX 9 SDK (June 2010).
# =============================================================================
param(
    [string]$RootDir   = (Resolve-Path "$PSScriptRoot\..\..").Path,
    [ValidateSet("x64", "Win32")]
    [string]$Arch      = "x64",
    [string]$BuildType = "Release",
    [string]$Target    = "",
    [switch]$LTO
)

$ErrorActionPreference = "Stop"
$BuildDir = if ($Arch -eq "x64") { Join-Path $RootDir "build-client" } else { Join-Path $RootDir "build-client-x86" }
$OutDir   = Join-Path $RootDir "env\client"
$LtoFlag  = if ($LTO) { "ON" } else { "OFF" }

Write-Host "[IrineCore] Configuring Windows ($Arch) Client & Launcher build (LTO=$LtoFlag)..." -ForegroundColor Cyan
cmake -S "$RootDir" -B "$BuildDir" -A $Arch `
    -DIRINE_BUILD_SERVERS=OFF `
    -DIRINE_BUILD_CLIENT=ON `
    -DIRINE_CLIENT_LTO=$LtoFlag `
    -DCMAKE_INSTALL_PREFIX="$OutDir"
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($LASTEXITCODE)" }

if ($Target) {
    Write-Host "[IrineCore] Building target $Target ($BuildType|$Arch)..." -ForegroundColor Cyan
    cmake --build "$BuildDir" --config $BuildType --target $Target --parallel -- /nr:false
    if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)" }
} else {
    Write-Host "[IrineCore] Building DnLauncher & DragonNest Client ($BuildType|$Arch)..." -ForegroundColor Cyan
    cmake --build "$BuildDir" --config $BuildType --parallel -- /nr:false
    if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)" }

    Write-Host "[IrineCore] Installing client binaries into $OutDir ..." -ForegroundColor Green
    cmake --install "$BuildDir" --config $BuildType
    if ($LASTEXITCODE -ne 0) { throw "Install failed ($LASTEXITCODE)" }
}
