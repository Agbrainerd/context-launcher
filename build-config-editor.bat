@echo off
REM Build script for Context Launcher Config Editor
REM Requires .NET Framework SDK (included with Visual Studio)

echo ==============================
echo Building Config Editor
echo ==============================
echo.

REM Try to find Roslyn compiler (VS 2015+) which supports C# 6.0
set "CSC_PATH="

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\Roslyn\csc.exe" (
    set "CSC_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\Roslyn\csc.exe"
    goto :found
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\Roslyn\csc.exe" (
    set "CSC_PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\Roslyn\csc.exe"
    goto :found
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\Roslyn\csc.exe" (
    set "CSC_PATH=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\Roslyn\csc.exe"
    goto :found
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\Roslyn\csc.exe" (
    set "CSC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\Roslyn\csc.exe"
    goto :found
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\Roslyn\csc.exe" (
    set "CSC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\Roslyn\csc.exe"
    goto :found
)

REM Fall back to old compiler in Windows
set "CSC_PATH=C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe"

:found
if not exist "%CSC_PATH%" (
    echo ERROR: C# compiler not found
    echo.
    echo Please install .NET Framework SDK or Visual Studio
    echo.
    if not "%1"=="--no-pause" pause
    exit /b 1
)

echo Using compiler: %CSC_PATH%
echo.

REM Compile
echo Compiling ConfigEditor.cs...
"%CSC_PATH%" /target:winexe /out:ConfigEditor.exe /r:System.dll /r:System.Windows.Forms.dll /r:System.Drawing.dll ConfigEditor.cs

if %errorlevel% neq 0 (
    echo.
    echo ==============================
    echo Build FAILED!
    echo ==============================
    if not "%1"=="--no-pause" pause
    exit /b %errorlevel%
) else (
    echo.
    echo ==============================
    echo Build successful!
    echo ==============================
    echo.
    echo ConfigEditor.exe has been created.
    echo.
    if not "%1"=="--no-pause" pause
)
