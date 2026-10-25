# Generates compile_commands.json for clangd.
#
# The main build uses the Visual Studio generator, which cannot export a compile
# database (CMake supports that only for Makefile/Ninja). So this script configures
# a *separate* Ninja build directory purely to produce one:
#
#   build/clangd/compile_commands.json  ->  <repo>/compile_commands.json
#
# Nothing is compiled; only the configure step runs. Re-run it after adding files or
# changing CMakeLists (or after pulling).
#
# Usage:
#   pwsh tools/gen-compile-db.ps1              # Debug, default build dir
#   pwsh tools/gen-compile-db.ps1 -Config Release
#   pwsh tools/gen-compile-db.ps1 -Fresh       # wipe the clangd build dir first

[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Config = "Debug",
    [string]$BuildDir = "build/clangd",
    [switch]$Fresh
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

function Find-VcVars {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw "vswhere.exe not found; install Visual Studio with the C++ workload"
    }
    $install = & $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath
    if (-not $install) {
        throw "No Visual Studio instance with the C++ toolset was found"
    }
    $vcvars = Join-Path $install "VC\Auxiliary\Build\vcvars64.bat"
    if (-not (Test-Path $vcvars)) {
        throw "vcvars64.bat not found under $install"
    }
    return $vcvars
}

$vcvars = Find-VcVars

if ($Fresh -and (Test-Path $BuildDir)) {
    Write-Host "Removing $BuildDir"
    Remove-Item -Recurse -Force $BuildDir
}

# Ninja needs the MSVC environment (cl.exe, INCLUDE, LIB); configure inside a shell
# that has it. CMake itself is found on PATH, so no absolute path is needed.
$ninja = Get-Command ninja -ErrorAction SilentlyContinue
$generatorArgs = "-G Ninja"
if (-not $ninja) {
    Write-Warning "ninja not on PATH; letting CMake pick the default generator (may not export a compile database)"
    $generatorArgs = ""
}

$cmakeArgs = @(
    "cmake -S `"$root`" -B `"$BuildDir`" $generatorArgs"
    "-DCMAKE_BUILD_TYPE=$Config"
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
) -join " "

# One command: the vcvars call must be followed by `&&` on the *shell* level, so the
# cmake invocation has to be a single string.
$configure = "call `"$vcvars`" >nul 2>&1 && $cmakeArgs"

Write-Host "Configuring $BuildDir ($Config) for clangd..."
cmd /c $configure
if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed (exit $LASTEXITCODE)"
}

$db = Join-Path $BuildDir "compile_commands.json"
if (-not (Test-Path $db)) {
    throw "$db was not produced. Ninja/Makefile generators are required for compile_commands.json"
}

Copy-Item $db "compile_commands.json" -Force
$count = (Get-Content "compile_commands.json" -Raw | ConvertFrom-Json).Count
Write-Host "compile_commands.json: $count entries (copied to the repo root)"
