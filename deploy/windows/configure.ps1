# =============================================================================
# IrineCore — deploy/windows/configure.ps1
# Validates env/dist/config/*.ini, env/dist/module/*.ini, and env/dist/systems/ChannelInfo.xml.
# =============================================================================
param(
    [string]$DistDir   = (Join-Path (Resolve-Path "$PSScriptRoot\..\..").Path "env\dist"),
    [string]$SqlServer = "127.0.0.1,1433"
)

$ErrorActionPreference = "Stop"
$RequiredServerInis = @("DNLog.ini", "DNDB.ini", "DNCash.ini", "DNLogin.ini", "DNMaster.ini", "DNVillage.ini", "DNGame.ini", "odbc.ini")
$RequiredModuleInis = @("Player-Bots.ini", "Hero-Ally.ini", "FPS-PING.ini")

foreach ($ini in $RequiredServerInis) {
    $path = Join-Path "$DistDir\config" $ini
    if (-not (Test-Path $path)) {
        throw "[IrineCore] Missing required server configuration file: $path"
    }
    Write-Host "[OK] Found config/$ini" -ForegroundColor Green
}

foreach ($ini in $RequiredModuleInis) {
    $path = Join-Path "$DistDir\module" $ini
    if (-not (Test-Path $path)) {
        throw "[IrineCore] Missing required module configuration file: $path"
    }
    Write-Host "[OK] Found module/$ini" -ForegroundColor Green
}

$channelXml = Join-Path "$DistDir\systems" "ChannelInfo.xml"
if (-not (Test-Path $channelXml)) {
    throw "[IrineCore] Missing required channel configuration: $channelXml"
}
Write-Host "[OK] Found systems/ChannelInfo.xml" -ForegroundColor Green
Write-Host "[IrineCore] Configuration validation succeeded." -ForegroundColor Cyan
