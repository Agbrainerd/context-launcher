@echo off
REM Master build script for Context Launcher
REM Builds both the launcher and config editor

echo ==============================
echo Context Launcher - Build All
echo ==============================
echo.

echo Building C++ Launcher...
call build.bat --no-pause
if %errorlevel% neq 0 (
    echo.
    echo Failed to build launcher!
    pause
    exit /b 1
)

echo.
echo Building C# Config Editor...
call build-config-editor.bat --no-pause
if %errorlevel% neq 0 (
    echo.
    echo Failed to build config editor!
    pause
    exit /b 1
)

echo.
echo ==============================
echo All builds successful!
echo ==============================
echo.
echo Files created:
echo   - context-launcher.exe
echo   - ConfigEditor.exe
echo.
echo Ready to create installer!
echo.
pause
