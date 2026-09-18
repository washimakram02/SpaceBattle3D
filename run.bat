@echo off
title 3D Space Defender Launcher
if not exist "SpaceDefender3D.exe" (
    echo [ERROR] SpaceDefender3D.exe not found!
    echo Please run build.bat first to compile the project.
    pause
    exit /b 1
)
start "" "SpaceDefender3D.exe"
