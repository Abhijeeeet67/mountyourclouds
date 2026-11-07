@echo off
REM ============================================================================
REM Brainrot Wars - Windows Build Script
REM Run from Developer Command Prompt for VS 2022
REM ============================================================================

set PROJECT_NAME=MultiverseHopper
set PROJECT_PATH=%CD%\%PROJECT_NAME%.uproject
set UE_VERSION=5.4

echo ============================================================================
echo Building %PROJECT_NAME% for Windows
echo ============================================================================

REM Detect UE5 installation
if not defined UE_ROOT (
    set UE_ROOT=C:\Program Files\Epic Games\UE_%UE_VERSION%
)
if not exist "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" (
    echo ERROR: UE5 not found at %UE_ROOT%
    echo Set UE_ROOT environment variable or install UE %UE_VERSION%
    exit /b 1
)

echo Using UE5 at: %UE_ROOT%

REM Generate project files
echo.
echo [1/4] Generating project files...
"%UE_ROOT%\Engine\Build\BatchFiles\GenerateProjectFiles.bat" "%PROJECT_PATH%" -2022
if errorlevel 1 (
    echo ERROR: GenerateProjectFiles failed
    exit /b 1
)

REM Build Editor (Development)
echo.
echo [2/4] Building Editor (Development)...
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" %PROJECT_NAME%Editor Win64 Development "%PROJECT_PATH%" -WaitMutex -FromMsBuild
if errorlevel 1 (
    echo ERROR: Editor build failed
    exit /b 1
)

REM Build Game (Shipping)
echo.
echo [3/4] Building Game (Shipping)...
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" %PROJECT_NAME% Win64 Shipping "%PROJECT_PATH%" -WaitMutex -FromMsBuild
if errorlevel 1 (
    echo ERROR: Game build failed
    exit /b 1
)

REM Optional: Package
echo.
echo [4/4] Build complete!
echo.
echo To package for distribution, run:
echo   "%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%PROJECT_PATH%" -noP4 -platform=Win64 -clientconfig=Shipping -cook -allmaps -build -stage -pak -prereqs -archive -archivedirectory="%CD%\Package\Windows" -utf8output -compile -compressed -distribution
echo.
echo Or run: PACKAGE_WINDOWS.bat
echo.
echo Output binaries in: %CD%\Binaries\Win64\
echo ============================================================================