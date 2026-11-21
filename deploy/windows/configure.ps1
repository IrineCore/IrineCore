# =============================================================================
# IrineCore — deploy/windows/configure.ps1
# Validates env/dist/config/*.ini, env/dist/module/*.ini, and env/dist/systems/ChannelInfo.xml.
# =============================================================================
param(
    [string]$DistDir   = (Join-Path (Resolve-Path "$PSScriptRoot\..\..").Path "env\dist"),
    [string]$SqlServer = "127.0.0.1,1433"
)

$ErrorActionPreference = "Stop"
$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
$RequiredServerInis = @("DNLog.ini", "DNDB.ini", "DNCash.ini", "DNLogin.ini", "DNMaster.ini", "DNVillage.ini", "DNGame.ini", "odbc.ini")

foreach ($ini in $RequiredServerInis) {
    $path = Join-Path "$DistDir\config" $ini
    if (-not (Test-Path $path)) {
        throw "[IrineCore] Missing required server configuration file: $path"
    }
    Write-Host "[OK] Found config/$ini" -ForegroundColor Green
}

# Auto-discover enabled module configs from modules.manifest (generated at install time)
# or fall back to scanning modules/<Mod>/{modules,conf}/*.{ini,conf,conf.dist}.
$ModuleConfigNames = @()
$manifestPath = Join-Path "$DistDir\module" "modules.manifest"
if (Test-Path $manifestPath) {
    $ModuleConfigNames = Get-Content $manifestPath | ForEach-Object { $_.Trim() } | Where-Object { $_ -and -not $_.StartsWith("#") }
} else {
    $modulesRoot = Join-Path $RepoRoot "modules"
    if (Test-Path $modulesRoot) {
        Get-ChildItem -Path $modulesRoot -Directory | Where-Object {
            $_.Name -ne "disabled" -and -not $_.Name.StartsWith(".") -and -not (Test-Path (Join-Path $_.FullName ".disabled"))
        } | ForEach-Object {
            foreach ($sub in @("modules", "conf")) {
                $subDir = Join-Path $_.FullName $sub
                if (Test-Path $subDir) {
                    Get-ChildItem -Path $subDir -File | Where-Object { $_.Name -match '\.(ini|conf|conf\.dist)$' } | ForEach-Object {
                        $ModuleConfigNames += $_.Name
                    }
                }
            }
        }
    }
}

foreach ($cfg in ($ModuleConfigNames | Select-Object -Unique)) {
    $path = Join-Path "$DistDir\module" $cfg
    if (-not (Test-Path $path)) {
        throw "[IrineCore] Missing enabled module configuration file: $path"
    }
    Write-Host "[OK] Found module/$cfg" -ForegroundColor Green
}

$channelXml = Join-Path "$DistDir\systems" "ChannelInfo.xml"
if (-not (Test-Path $channelXml)) {
    throw "[IrineCore] Missing required channel configuration: $channelXml"
}
Write-Host "[OK] Found systems/ChannelInfo.xml" -ForegroundColor Green
Write-Host "[IrineCore] Configuration validation succeeded." -ForegroundColor Cyan
