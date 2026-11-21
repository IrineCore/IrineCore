# =============================================================================
# IrineCore — apps/docker/deploy-all.ps1
# Builds & starts the complete 9-container IrineCore stack (auto-populated
# MSSQL from data/sql/base + Nginx + all 7 server daemons).
# =============================================================================
param(
    [string]$RootDir = (Resolve-Path "$PSScriptRoot\..\..").Path
)

$ErrorActionPreference = "Stop"

Push-Location $RootDir
try {
    foreach ($svcDir in @("deploy\mssql", "deploy\webserver")) {
        $envPath = Join-Path $RootDir "$svcDir\.env"
        $examplePath = Join-Path $RootDir "$svcDir\.env.example"
        if (-not (Test-Path $envPath) -and (Test-Path $examplePath)) {
            Copy-Item $examplePath $envPath
            Write-Host "[IrineCore] Created $svcDir\.env from .env.example" -ForegroundColor Yellow
        }
    }

    Write-Host "[IrineCore] Building and starting complete 9-container cluster..." -ForegroundColor Cyan
    docker compose up -d --build
    if ($LASTEXITCODE -ne 0) { throw "docker compose up failed." }
    docker compose ps
}
finally {
    Pop-Location
}
