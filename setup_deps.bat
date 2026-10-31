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
echo [1/6] Cloning GLM...
if not exist "glm" (
    git clone https://github.com/mallocobject/glm.git glm
    if errorlevel 1 (
        echo [ERROR] Failed to clone GLM
        goto :error
    )
) else (
    echo [SKIP] glm already exists
)

echo.
echo [2/6] Cloning ImGui (docking branch)...
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
echo [3/6] Cloning BqLog...
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
echo [4/6] Cloning SDL3 (window/input back end)...
if not exist "SDL" (
    git clone https://github.com/mallocobject/SDL.git SDL
    if errorlevel 1 (
        echo [ERROR] Failed to clone SDL3
        goto :error
    )
) else (
    echo [SKIP] SDL already exists
)

echo.
echo [5/6] Cloning ImGuizmo...
if not exist "ImGuizmo" (
    git clone https://github.com/mallocobject/ImGuizmo.git ImGuizmo
    if errorlevel 1 (
        echo [ERROR] Failed to clone ImGuizmo
        goto :error
    )
) else (
    echo [SKIP] ImGuizmo already exists
)

echo.
echo [6/6] Cloning yaml-cpp...
if not exist "yaml-cpp" (
    git clone https://github.com/mallocobject/yaml-cpp.git yaml-cpp
    if errorlevel 1 (
        echo [ERROR] Failed to clone yaml-cpp
        goto :error
    )
) else (
    echo [SKIP] yaml-cpp already exists
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
