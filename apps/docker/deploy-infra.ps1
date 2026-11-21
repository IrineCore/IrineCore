# =============================================================================
# IrineCore — apps/docker/deploy-infra.ps1
# Starts Dockerized MSSQL (auto-populates all 5 DBs directly from data/sql/base
# + data/sql/updates) and Nginx Patch WebServer, waiting until healthy.
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

    Write-Host "[IrineCore] Building and starting irine-database and irine-webserver..." -ForegroundColor Cyan
    docker compose up -d --build --wait irine-database irine-webserver
    if ($LASTEXITCODE -ne 0) { throw "docker compose up failed." }

    Write-Host "[IrineCore] Infrastructure stack is populated and healthy." -ForegroundColor Green
    Write-Host "  - SQL Server  : 127.0.0.1,1433 (dngsm, dnmembership, dnserverlog, dnstaging, dnworld)"
    Write-Host "  - Patch HTTP  : http://127.0.0.1/Game/DragonNest/patch/PatchInfoServer.cfg"
    Write-Host "  - HealthCheck : http://127.0.0.1/health"
}
finally {
    Pop-Location
}
