# Build script for IDS_DEStruct using MSVC + Qt qmake.
# Usage:
#   .\build-msvc.ps1
#   .\build-msvc.ps1 -Debug
#   .\build-msvc.ps1 -Clean

param(
    [switch]$Clean,
    [switch]$Debug
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

Write-Host "=== Build IDS_DEStruct (MSVC) ===" -ForegroundColor Cyan

$projectRoot = $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($projectRoot)) {
    $projectRoot = (Get-Location).Path
}

$projectFile = Join-Path $projectRoot "IDS_DEStruct.pro"
if (-not (Test-Path -LiteralPath $projectFile)) {
    Write-Host "ERROR: Run this script from the project root (IDS_DEStruct.pro not found)." -ForegroundColor Red
    exit 1
}

$buildMode = if ($Debug) { "debug" } else { "release" }
$buildDir = Join-Path $projectRoot "build"
Write-Host "Mode: $buildMode" -ForegroundColor Yellow

if ($Clean -and (Test-Path -LiteralPath $buildDir)) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -LiteralPath $buildDir -Recurse -Force
}

if (-not (Test-Path -LiteralPath $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

$vcvarsCandidates = @(
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat",
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

$vcvarsPath = $vcvarsCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if (-not $vcvarsPath) {
    Write-Host "ERROR: Could not find vcvars64.bat (Visual Studio Build Tools)." -ForegroundColor Red
    exit 1
}

Write-Host "Configuring MSVC environment..." -ForegroundColor Yellow
$tempBat = [System.IO.Path]::GetTempFileName() + ".bat"
$tempOut = [System.IO.Path]::GetTempFileName()

@"
@echo off
call "$vcvarsPath" >nul 2>&1
if errorlevel 1 exit /b 1
set > "$tempOut"
"@ | Set-Content -LiteralPath $tempBat -Encoding ASCII

cmd.exe /d /c "`"$tempBat`""
$vcvarsExit = $LASTEXITCODE
Remove-Item -LiteralPath $tempBat -Force

if ($vcvarsExit -ne 0) {
    if (Test-Path -LiteralPath $tempOut) {
        Remove-Item -LiteralPath $tempOut -Force
    }
    Write-Host "ERROR: Failed to initialize MSVC environment." -ForegroundColor Red
    exit 1
}

Get-Content -LiteralPath $tempOut | ForEach-Object {
    if ($_ -match "^([^=]+)=(.*)$") {
        [System.Environment]::SetEnvironmentVariable(
            $matches[1],
            $matches[2],
            [System.EnvironmentVariableTarget]::Process
        )
    }
}
Remove-Item -LiteralPath $tempOut -Force

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    Write-Host "ERROR: cl.exe not found after vcvars initialization." -ForegroundColor Red
    exit 1
}

if (-not (Get-Command nmake.exe -ErrorAction SilentlyContinue)) {
    Write-Host "ERROR: nmake.exe not found after vcvars initialization." -ForegroundColor Red
    exit 1
}

$qmakeCandidates = New-Object System.Collections.Generic.List[string]
$qmakeOnPath = Get-Command qmake.exe -ErrorAction SilentlyContinue
if ($qmakeOnPath) {
    $qmakeCandidates.Add($qmakeOnPath.Source)
}

if ($env:QTDIR) {
    $qmakeFromQtDir = Join-Path $env:QTDIR "bin\qmake.exe"
    if (Test-Path -LiteralPath $qmakeFromQtDir) {
        $qmakeCandidates.Add($qmakeFromQtDir)
    }
}

$qmakeCandidates.Add("C:\Qt\5.15.2\msvc2019_64\bin\qmake.exe")
$qmakeCandidates.Add("C:\Qt\5.15.2\msvc2022_64\bin\qmake.exe")
$qmakeCandidates.Add("C:\Qt\6.5.3\msvc2019_64\bin\qmake.exe")
$qmakeCandidates.Add("C:\Qt\6.5.3\msvc2022_64\bin\qmake.exe")

$qmakeExe = $null
foreach ($candidate in ($qmakeCandidates | Select-Object -Unique)) {
    if (-not (Test-Path -LiteralPath $candidate)) {
        continue
    }

    $spec = & $candidate -query QMAKE_XSPEC 2>$null
    if (-not $spec) {
        $spec = & $candidate -query QMAKE_SPEC 2>$null
    }

    if ($spec -match "msvc") {
        $qmakeExe = $candidate
        break
    }
}

if (-not $qmakeExe) {
    Write-Host "ERROR: qmake for MSVC was not found. Install a Qt MSVC kit or update this script." -ForegroundColor Red
    exit 1
}

Write-Host "Using qmake: $qmakeExe" -ForegroundColor Green

Push-Location $buildDir
try {
    Write-Host "`nRunning qmake..." -ForegroundColor Yellow
    & $qmakeExe $projectFile "CONFIG+=$buildMode"
    if ($LASTEXITCODE -ne 0) {
        throw "qmake failed with exit code $LASTEXITCODE."
    }

    Write-Host "qmake finished." -ForegroundColor Green
    Write-Host "`nBuilding with nmake..." -ForegroundColor Yellow
    & nmake /NOLOGO
    if ($LASTEXITCODE -ne 0) {
        throw "nmake failed with exit code $LASTEXITCODE."
    }

    Write-Host "`nBuild finished successfully." -ForegroundColor Green

    $exeCandidates = @(
        (Join-Path $buildDir "$buildMode\IDS_DEStruct.exe"),
        (Join-Path $buildDir "$buildMode\IDS_DEStructd.exe"),
        (Join-Path $buildDir "release\IDS_DEStruct.exe"),
        (Join-Path $buildDir "debug\IDS_DEStructd.exe"),
        (Join-Path $buildDir "IDS_DEStruct.exe")
    )

    foreach ($exePath in $exeCandidates) {
        if (Test-Path -LiteralPath $exePath) {
            $fullPath = (Resolve-Path -LiteralPath $exePath).Path
            Write-Host "Executable: $fullPath" -ForegroundColor Cyan
            break
        }
    }
}
catch {
    Write-Host ("`nERROR during build: {0}" -f $_.Exception.Message) -ForegroundColor Red
    exit 1
}
finally {
    Pop-Location
}
