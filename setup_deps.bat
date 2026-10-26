@echo off
setlocal

set "ROOT=%~dp0"
set "DEPS=%ROOT%deps"

echo ========================================
echo  IVX Dependency Setup
echo ========================================
echo.

if not exist "%DEPS%" (
    echo Creating deps directory...
    mkdir "%DEPS%"
)

cd /d "%DEPS%"

echo.
echo [1/4] Cloning GLM...
if not exist "glm" (
    git clone https://github.com/icaven/glm.git glm
    if errorlevel 1 (
        echo [ERROR] Failed to clone GLM
        goto :error
    )
) else (
    echo [SKIP] glm already exists
)

echo.
echo [2/4] Cloning ImGui (docking branch)...
if not exist "imgui" (
    git clone -b docking https://github.com/mallocobject/imgui.git imgui
    if errorlevel 1 (
        echo [ERROR] Failed to clone ImGui
        goto :error
    )
) else (
    echo [SKIP] imgui already exists
)

echo.
echo [3/4] Cloning BqLog...
if not exist "BqLog" (
    git clone https://github.com/mallocobject/BqLog.git BqLog
    if errorlevel 1 (
        echo [ERROR] Failed to clone BqLog
        goto :error
    )
) else (
    echo [SKIP] BqLog already exists
)

echo.
echo [4/4] Cloning SDL3 (window/input back end)...
if not exist "SDL" (
    git clone -b release-3.4.0 https://github.com/libsdl-org/SDL.git SDL
    if errorlevel 1 (
        echo [ERROR] Failed to clone SDL3
        goto :error
    )
) else (
    echo [SKIP] SDL already exists
)

echo.
echo ========================================
echo  All dependencies are ready in:
echo  %DEPS%
echo ========================================
echo.
pause
exit /b 0

:error
echo.
echo ========================================
echo  Setup failed. Please check the errors.
echo ========================================
pause
exit /b 1
