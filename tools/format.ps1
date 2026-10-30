# Runs clang-format over the first-party C++ sources.
#
# Vendor/deps and the generated build trees are skipped on purpose: reformatting
# third-party code would bury real changes in noise.
#
# Usage:
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools/format.ps1            # rewrite files in place
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools/format.ps1 -Check     # only report files that need formatting

[CmdletBinding()]
param(
    [switch]$Check
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$candidates = @(
    "${env:ProgramFiles}\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\clang-format.exe"
    "${env:ProgramFiles}\Microsoft Visual Studio\17\Community\VC\Tools\Llvm\x64\bin\clang-format.exe"
)
$clangFormat = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $clangFormat) {
    $fromPath = Get-Command clang-format -ErrorAction SilentlyContinue
    if ($fromPath) {
        $clangFormat = $fromPath.Source
    } else {
        throw "clang-format not found (install LLVM or the Visual Studio C++ workload)"
    }
}

$dirs = @("IVX/src", "VXEdit/src", "VXEdit/scripts", "Sandbox/src")
$files = foreach ($dir in $dirs) {
    if (Test-Path $dir) {
        Get-ChildItem $dir -Recurse -Include *.cpp, *.h, *.hpp
    }
}

Write-Host "clang-format: $clangFormat"
Write-Host "files: $($files.Count)"

if ($Check) {
    $needsWork = @()
    foreach ($file in $files) {
        & $clangFormat --dry-run --Werror --style=file $file.FullName 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) {
            $needsWork += $file.FullName
        }
    }
    if ($needsWork.Count -eq 0) {
        Write-Host "all files formatted"
        exit 0
    }
    $needsWork | ForEach-Object { Write-Host "  needs formatting: $_" }
    Write-Host "$($needsWork.Count) file(s) need formatting"
    exit 1
}

foreach ($file in $files) {
    & $clangFormat -i --style=file $file.FullName
}
Write-Host "done"
