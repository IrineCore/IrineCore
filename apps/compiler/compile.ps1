# =============================================================================
# IrineCore — apps/compiler/compile.ps1
# Configures, builds, runs CTest, and installs IrineCore into env/dist.
# Auto-loads conf\dist\config.ps1 and optional local override conf\config.ps1.
# =============================================================================
param(
    [string]$RootDir   = (Resolve-Path "$PSScriptRoot\..\..").Path,
    [string]$BuildType = ""
)

$ErrorActionPreference = "Stop"

# Load tracked defaults, then optional user override
$distConf  = Join-Path $RootDir "conf\dist\config.ps1"
$localConf = Join-Path $RootDir "conf\config.ps1"
if (Test-Path $distConf)  { . $distConf }
if (Test-Path $localConf) { . $localConf }

function Resolve-RepoPath([string]$p, [string]$fallback) {
    $val = if ($p) { $p } else { $fallback }
    if ([System.IO.Path]::IsPathRooted($val)) { return $val }
    return (Join-Path $RootDir $val)
}

if (-not $BuildType) { $BuildType = if ($IRINE_BUILD_TYPE) { $IRINE_BUILD_TYPE } else { "Release" } }
$BuildDir = Resolve-RepoPath $IRINE_BUILD_DIR "build"
$DistDir  = Resolve-RepoPath $IRINE_DIST_DIR  "env\dist"

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

Write-Host "[IrineCore] Configuring CMake project ($BuildType)..." -ForegroundColor Cyan
cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[IrineCore] Compiling all libraries, modules, and server applications ($BuildType)..." -ForegroundColor Cyan
cmake --build $BuildDir --config $BuildType --parallel -- /nr:false
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[IrineCore] Running automated test suite..." -ForegroundColor Cyan
ctest --test-dir $BuildDir -C $BuildType --output-on-failure
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "[IrineCore] Installing binaries and configurations into $DistDir ..." -ForegroundColor Green
cmake --install $BuildDir --config $BuildType
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
