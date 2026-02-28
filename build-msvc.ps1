# Build script for IDS_DEStruct using MSVC + Qt qmake.
# Usage: .\build-msvc.ps1 [-Clean] [-Debug]

param(
    [switch]$Clean,
    [switch]$Debug
)

Write-Host "=== Build IDS_DEStruct (MSVC) ===" -ForegroundColor Cyan

if (-not (Test-Path "IDS_DEStruct.pro")) {
    Write-Host "ERROR: Run this script from the project root." -ForegroundColor Red
    exit 1
}

$buildMode = if ($Debug) { "debug" } else { "release" }
$buildDir = "build"
Write-Host "Mode: $buildMode" -ForegroundColor Yellow

if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Path $buildDir -Recurse -Force
}

if (-not (Test-Path $buildDir)) {
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

$vcvarsPath = $vcvarsCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $vcvarsPath) {
    Write-Host "ERROR: Could not find vcvars64.bat (Visual Studio Build Tools)." -ForegroundColor Red
    exit 1
}

Write-Host "Configuring MSVC environment..." -ForegroundColor Yellow
$tempBat = [System.IO.Path]::GetTempFileName() + ".bat"
$tempOut = [System.IO.Path]::GetTempFileName()

@"
@echo off
call "$vcvarsPath"
set > "$tempOut"
"@ | Out-File -FilePath $tempBat -Encoding ASCII

cmd.exe /c $tempBat | Out-Null
$vcvarsExit = $LASTEXITCODE
Remove-Item $tempBat -Force

if ($vcvarsExit -ne 0) {
    Write-Host "ERROR: Failed to initialize MSVC environment." -ForegroundColor Red
    if (Test-Path $tempOut) { Remove-Item $tempOut -Force }
    exit 1
}

Get-Content $tempOut | ForEach-Object {
    if ($_ -match "^([^=]+)=(.*)$") {
        [System.Environment]::SetEnvironmentVariable(
            $matches[1],
            $matches[2],
            [System.EnvironmentVariableTarget]::Process
        )
    }
}
Remove-Item $tempOut -Force
Write-Host "MSVC environment ready." -ForegroundColor Green

$qmakeCommand = Get-Command qmake -ErrorAction SilentlyContinue
$qmakeExe = if ($qmakeCommand) {
    $qmakeCommand.Source
} else {
    $qmakeCandidates = @(
        "C:\Qt\5.15.2\msvc2019_64\bin\qmake.exe",
        "C:\Qt\5.15.2\msvc2022_64\bin\qmake.exe"
    )
    $qmakeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

if (-not $qmakeExe) {
    Write-Host "ERROR: qmake not found. Add Qt bin to PATH or adjust this script." -ForegroundColor Red
    exit 1
}

Push-Location $buildDir
try {
    Write-Host "`nRunning qmake..." -ForegroundColor Yellow
    & $qmakeExe "..\IDS_DEStruct.pro" "CONFIG+=$buildMode"
    if ($LASTEXITCODE -ne 0) {
        throw "qmake failed."
    }
    Write-Host "qmake finished." -ForegroundColor Green

    Write-Host "`nBuilding with nmake..." -ForegroundColor Yellow
    & nmake
    if ($LASTEXITCODE -ne 0) {
        throw "nmake failed."
    }

    Write-Host "`nBuild finished successfully." -ForegroundColor Green

    $exeCandidates = @(
        "$buildMode\IDS_DEStruct.exe",
        "$buildMode\IDS_DEStructd.exe",
        "release\IDS_DEStruct.exe",
        "debug\IDS_DEStruct.exe",
        "IDS_DEStruct.exe"
    )

    foreach ($exePath in $exeCandidates) {
        if (Test-Path $exePath) {
            $fullPath = (Resolve-Path $exePath).Path
            Write-Host "`nExecutable: $fullPath" -ForegroundColor Cyan
            break
        }
    }
}
catch {
    Write-Host ("`nERROR during build: {0}" -f $_) -ForegroundColor Red
    exit 1
}
finally {
    Pop-Location
}
