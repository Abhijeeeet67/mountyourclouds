@echo off
REM ============================================================================
REM Brainrot Wars - Windows Packaging Script
REM Run from Developer Command Prompt for VS 2022
REM ============================================================================

set PROJECT_NAME=MultiverseHopper
set PROJECT_PATH=%CD%\%PROJECT_NAME%.uproject
set UE_VERSION=5.4
set CONFIG=Shipping

echo ============================================================================
echo Packaging %PROJECT_NAME% for Windows (%CONFIG%)
echo ============================================================================

if not defined UE_ROOT (
    set UE_ROOT=C:\Program Files\Epic Games\UE_%UE_VERSION%
)
if not exist "%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" (
    echo ERROR: UE5 not found at %UE_ROOT%
    exit /b 1
)

echo Using UE5 at: %UE_ROOT%

REM Clean previous package
if exist "%CD%\Package\Windows" (
    echo Cleaning previous package...
    rmdir /s /q "%CD%\Package\Windows"
)

REM RunUAT BuildCookRun
"%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun `
    -project="%PROJECT_PATH%" `
    -noP4 `
    -platform=Win64 `
    -clientconfig=%CONFIG% `
    -serverconfig=%CONFIG% `
    -cook -allmaps `
    -build -stage -pak `
    -prereqs `
    -archive `
    -archivedirectory="%CD%\Package\Windows" `
    -utf8output -compile -compressed -distribution

if errorlevel 1 (
    echo.
    echo ERROR: Packaging failed!
    exit /b 1
)

echo.
echo ============================================================================
echo PACKAGING COMPLETE!
echo ============================================================================
echo Output: %CD%\Package\Windows\WindowsNoEditor\%PROJECT_NAME%\
echo Executable: %CD%\Package\Windows\WindowsNoEditor\%PROJECT_NAME%\Binaries\Win64\%PROJECT_NAME%-Win64-Shipping.exe
echo.
echo To create a zip for distribution:
echo   powershell Compress-Archive -Path "%CD%\Package\Windows\WindowsNoEditor\%PROJECT_NAME%\*" -DestinationPath "%CD%\MultiverseHopper-Windows-%CONFIG%.zip" -Force
echo ============================================================================