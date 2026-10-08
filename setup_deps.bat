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
echo [1/3] Cloning GLFW...
if not exist "glfw" (
    git clone https://github.com/mallocobject/glfw.git glfw
    if errorlevel 1 (
        echo [ERROR] Failed to clone GLFW
        goto :error
    )
) else (
    echo [SKIP] glfw already exists
)

echo.
echo [2/3] Cloning GLM...
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
echo [3/3] Cloning ImGui (docking branch)...
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