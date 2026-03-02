# Script de build com MSVC 2019 para IDS_DEStruct
# Uso: .\build-msvc.ps1 [-Clean] [-Debug]

param(
    [switch]$Clean,
    [switch]$Debug
)

Write-Host "=== Build IDS_DEStruct com MSVC 2019 ===" -ForegroundColor Cyan

# Verificar se está no diretório correto
if (-not (Test-Path "IDS_DEStruct.pro")) {
    Write-Host "❌ Erro: Execute este script na raiz do projeto" -ForegroundColor Red
    exit 1
}

# Configuração
$buildMode = if ($Debug) { "debug" } else { "release" }
$buildDir = "build"

Write-Host "Modo: $buildMode" -ForegroundColor Yellow

# Limpar build se solicitado
if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "Limpando diretório de build..." -ForegroundColor Yellow
    Remove-Item -Path $buildDir -Recurse -Force
}

# Criar diretório de build
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

# Caminho do vcvars64.bat (ajuste se necessário)
$vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if (-not (Test-Path $vcvarsPath)) {
    # Tentar caminho alternativo (Community/Professional/Enterprise)
    $vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
}

if (-not (Test-Path $vcvarsPath)) {
    $vcvarsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
}

if (-not (Test-Path $vcvarsPath)) {
    Write-Host "❌ MSVC 2019 não encontrado" -ForegroundColor Red
    Write-Host "Instale Visual Studio 2019 Build Tools ou ajuste o caminho no script" -ForegroundColor Yellow
    exit 1
}

Write-Host "Configurando ambiente MSVC..." -ForegroundColor Yellow

# Executar vcvars64.bat e capturar variáveis de ambiente
$tempBat = [System.IO.Path]::GetTempFileName() + ".bat"
$tempOut = [System.IO.Path]::GetTempFileName()

@"
@echo off
call "$vcvarsPath"
set > "$tempOut"
"@ | Out-File -FilePath $tempBat -Encoding ASCII

cmd.exe /c $tempBat | Out-Null
Remove-Item $tempBat

# Importar variáveis de ambiente
Get-Content $tempOut | ForEach-Object {
    if ($_ -match "^([^=]+)=(.*)$") {
        [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], [System.EnvironmentVariableTarget]::Process)
    }
}
Remove-Item $tempOut

Write-Host "✔ Ambiente MSVC configurado" -ForegroundColor Green

# Executar qmake
Write-Host "`nExecutando qmake..." -ForegroundColor Yellow
Push-Location $buildDir
try {
    $qmakeArgs = @("..\IDS_DEStruct.pro", "CONFIG+=$buildMode")
    & qmake $qmakeArgs
    
    if ($LASTEXITCODE -ne 0) {
        throw "qmake falhou"
    }
    
    Write-Host "✔ qmake concluído" -ForegroundColor Green
    
    # Executar nmake
    Write-Host "`nCompilando com nmake..." -ForegroundColor Yellow
    & nmake
    
    if ($LASTEXITCODE -ne 0) {
        throw "nmake falhou"
    }
    
    Write-Host "`n✔ Build concluído com sucesso!" -ForegroundColor Green
    
    # Localizar executável
    $exePaths = @(
        "$buildMode\IDS_DEStruct.exe",
        "$buildMode\IDS_DEStructd.exe",
        "IDS_DEStruct.exe"
    )
    
    foreach ($exePath in $exePaths) {
        if (Test-Path $exePath) {
            $fullPath = (Resolve-Path $exePath).Path
            Write-Host "`nExecutável: $fullPath" -ForegroundColor Cyan
            break
        }
    }
    
} catch {
    Write-Host "`n❌ Erro durante o build: $_" -ForegroundColor Red
    exit 1
} finally {
    Pop-Location
}
