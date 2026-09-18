@echo off
setlocal
title Building 3D Space Defender

echo ========================================================
echo       Building 3D Space Defender (OpenGL / GLUT)
echo ========================================================

:: Check for CodeBlocks MinGW or system MinGW in PATH
where g++ >nul 2>nul
if %errorlevel% equ 0 goto COMPILE

set "CB_MINGW=C:\Program Files (x86)\CodeBlocks\MinGW\bin"
if exist "%CB_MINGW%\g++.exe" (
    echo [INFO] Found CodeBlocks MinGW at "%CB_MINGW%"
    set "PATH=%CB_MINGW%;%PATH%"
    goto COMPILE
)

if exist "C:\MinGW\bin\g++.exe" (
    echo [INFO] Found MinGW at C:\MinGW\bin
    set "PATH=C:\MinGW\bin;%PATH%"
    goto COMPILE
)

echo [ERROR] Could not find g++.exe in PATH or standard locations!
echo Please ensure MinGW or Code::Blocks is installed.
pause
exit /b 1

:COMPILE
echo [1/2] Compiling C++ source files...
g++ -std=c++11 -O2 -Wall -I.\include -L.\lib main.cpp Game.cpp -o SpaceDefender3D.exe -lglut32 -lglu32 -lopengl32 -lwinmm

if %errorlevel% equ 0 (
    echo.
    echo ========================================================
    echo [SUCCESS] Build completed successfully!
    echo Output: SpaceDefender3D.exe
    echo ========================================================
    echo You can now run "run.bat" or launch "SpaceDefender3D.exe".
) else (
    echo.
    echo [FAILED] Compilation errors encountered.
)

pause
