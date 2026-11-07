#!/bin/bash
# ============================================================================
# Brainrot Wars - Linux Packaging Script
# Run in Ubuntu 22.04+ with UE5 installed at /opt/unrealengine
# ============================================================================

set -euo pipefail

PROJECT_NAME="MultiverseHopper"
PROJECT_PATH="$(pwd)/${PROJECT_NAME}.uproject"
UE_ROOT="/opt/unrealengine"
CONFIG="Shipping"

echo "============================================================================"
echo "Packaging ${PROJECT_NAME} for Linux (${CONFIG})"
echo "============================================================================"

if [[ ! -f "${UE_ROOT}/Engine/Build/BatchFiles/Linux/RunUAT.sh" ]]; then
    echo "ERROR: UE5 not found at ${UE_ROOT}"
    exit 1
fi

echo "Using UE5 at: ${UE_ROOT}"

# Clean previous package
if [[ -d "$(pwd)/Package/Linux" ]]; then
    echo "Cleaning previous package..."
    rm -rf "$(pwd)/Package/Linux"
fi

# RunUAT BuildCookRun
"${UE_ROOT}/Engine/Build/BatchFiles/Linux/RunUAT.sh" BuildCookRun \
    -project="${PROJECT_PATH}" \
    -noP4 \
    -platform=Linux \
    -clientconfig=${CONFIG} \
    -serverconfig=${CONFIG} \
    -cook -allmaps \
    -build -stage -pak \
    -archive \
    -archivedirectory="$(pwd)/Package/Linux" \
    -utf8output -compile -compressed -distribution

if [[ $? -ne 0 ]]; then
    echo ""
    echo "ERROR: Packaging failed!"
    exit 1
fi

echo ""
echo "============================================================================"
echo "PACKAGING COMPLETE!"
echo "============================================================================"
echo "Output: $(pwd)/Package/Linux/LinuxNoEditor/${PROJECT_NAME}/"
echo "Executable: $(pwd)/Package/Linux/LinuxNoEditor/${PROJECT_NAME}/Binaries/Linux/${PROJECT_NAME}"
echo ""
echo "To create a tarball for distribution:"
echo "  tar -czf MultiverseHopper-Linux-${CONFIG}.tar.gz -C $(pwd)/Package/Linux/LinuxNoEditor ${PROJECT_NAME}"
echo "============================================================================"