#!/bin/bash
# ============================================================================
# Brainrot Wars - Linux Build Script
# Run in Ubuntu 22.04+ with UE5 installed at /opt/unrealengine
# ============================================================================

set -euo pipefail

PROJECT_NAME="MultiverseHopper"
PROJECT_PATH="$(pwd)/${PROJECT_NAME}.uproject"
UE_VERSION="5.4"
UE_ROOT="/opt/unrealengine"

echo "============================================================================"
echo "Building ${PROJECT_NAME} for Linux"
echo "============================================================================"

# Check UE5 installation
if [[ ! -f "${UE_ROOT}/Engine/Build/BatchFiles/Linux/Build.sh" ]]; then
    echo "ERROR: UE5 not found at ${UE_ROOT}"
    echo "Install UE5 or set UE_ROOT environment variable"
    exit 1
fi

echo "Using UE5 at: ${UE_ROOT}"

# Generate project files
echo ""
echo "[1/3] Generating project files..."
"${UE_ROOT}/Engine/Build/BatchFiles/Linux/GenerateProjectFiles.sh" "${PROJECT_PATH}"
if [[ $? -ne 0 ]]; then
    echo "ERROR: GenerateProjectFiles failed"
    exit 1
fi

# Build Editor (Development)
echo ""
echo "[2/3] Building Editor (Development)..."
"${UE_ROOT}/Engine/Build/BatchFiles/Linux/Build.sh" "${PROJECT_NAME}Editor" Linux Development "${PROJECT_PATH}" -WaitMutex
if [[ $? -ne 0 ]]; then
    echo "ERROR: Editor build failed"
    exit 1
fi

# Build Game (Shipping)
echo ""
echo "[3/3] Building Game (Shipping)..."
"${UE_ROOT}/Engine/Build/BatchFiles/Linux/Build.sh" "${PROJECT_NAME}" Linux Shipping "${PROJECT_PATH}" -WaitMutex
if [[ $? -ne 0 ]]; then
    echo "ERROR: Game build failed"
    exit 1
fi

echo ""
echo "============================================================================"
echo "BUILD COMPLETE!"
echo "============================================================================"
echo "Editor:  $(pwd)/Binaries/Linux/${PROJECT_NAME}Editor"
echo "Game:    $(pwd)/Binaries/Linux/${PROJECT_NAME}"
echo ""
echo "To package for distribution, run: ./PACKAGE_LINUX.sh"
echo "============================================================================"