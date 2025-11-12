@echo off
REM Build script for Context Launcher
REM Auto-detects Visual Studio installation

echo ==============================
echo Building Context Launcher
echo ==============================
echo.

REM Check if we're already in a VS developer environment
where cl.exe >nul 2>&1
if %errorlevel% equ 0 (
    echo Using existing Visual Studio environment...
    goto :compile
)

REM Try to find and setup Visual Studio environment
echo Looking for Visual Studio...

REM VS 2022
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    echo Found Visual Studio 2022 Community
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" >nul
    goto :compile
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" (
    echo Found Visual Studio 2022 Professional
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" >nul
    goto :compile
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" (
    echo Found Visual Studio 2022 Enterprise
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" >nul
    goto :compile
)

REM VS 2019
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" (
    echo Found Visual Studio 2019 Community
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" >nul
    goto :compile
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat" (
    echo Found Visual Studio 2019 Professional
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat" >nul
    goto :compile
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" (
    echo Found Visual Studio 2019 Enterprise
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\Tools\VsDevCmd.bat" >nul
    goto :compile
)

echo.
echo ERROR: Visual Studio not found!
echo.
echo Please install Visual Studio 2019 or 2022 with C++ Desktop Development workload
echo Or run this from "Developer Command Prompt for VS"
echo.
pause
exit /b 1

:compile
echo.
REM Compile
echo Compiling launcher.cpp...
cl /EHsc /O2 /Fe:context-launcher.exe launcher.cpp ole32.lib oleaut32.lib shlwapi.lib shell32.lib user32.lib

if %errorlevel% equ 0 (
    echo.
    echo ==============================
    echo Build successful!
    echo ==============================
    echo.
    echo context-launcher.exe has been created.
    echo.
    
    REM Clean up intermediate files
    del *.obj >nul 2>nul
) else (
    echo.
    echo ==============================
    echo Build FAILED!
    echo ==============================
    if not "%1"=="--no-pause" pause
    exit /b %errorlevel%
)

if not "%1"=="--no-pause" pause
