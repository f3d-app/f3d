#!/usr/bin/env bash

# cd to the root directory of the project
pushd "$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)/.."

set -euo pipefail

# Usage: scripts/build_macos.sh [Release|Debug]
BUILD_TYPE=${1:-Release}

ROOT_DIR=$(pwd)
BUILD_DIR="${ROOT_DIR}/build_macos/.build_tmp"
INSTALL_DIR="${ROOT_DIR}/build_macos/install"

# Clean staging and tmp build dir each run; keep final install dir stable path
rm -rf "${BUILD_DIR}" "${INSTALL_DIR}"
mkdir -p "${BUILD_DIR}" "${INSTALL_DIR}"

# Optional: pass dependency search directories to help copy non-system dylibs
# Set F3D_DEPENDENCIES_DIR to a semicolon-separated list if needed, e.g.:
#   export F3D_DEPENDENCIES_DIR="/opt/homebrew/opt/vtk/lib;/opt/homebrew/lib"

# Configure for an official macOS bundle into INSTALL_DIR
CMAKE_ARGS=()
CMAKE_ARGS+=(-S "${ROOT_DIR}" -B "${BUILD_DIR}")
CMAKE_ARGS+=(-DCMAKE_BUILD_TYPE="${BUILD_TYPE}")
CMAKE_ARGS+=(-DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}")
CMAKE_ARGS+=(-DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON)
CMAKE_ARGS+=(-DCMAKE_OSX_DEPLOYMENT_TARGET=12.0)
CMAKE_ARGS+=(-DBUILD_SHARED_LIBS=ON)
CMAKE_ARGS+=(-DF3D_MACOS_BUNDLE=ON)
CMAKE_ARGS+=(-DF3D_PLUGINS_STATIC_BUILD=ON)
# Install all libraries into the bundle Frameworks directory
CMAKE_ARGS+=(-DCMAKE_INSTALL_LIBDIR="f3d.app/Contents/Frameworks")
if [[ -n "${F3D_DEPENDENCIES_DIR:-}" ]]; then
  CMAKE_ARGS+=(-DF3D_DEPENDENCIES_DIR="${F3D_DEPENDENCIES_DIR}")
fi
cmake "${CMAKE_ARGS[@]}"

# Build
cmake --build "${BUILD_DIR}" --parallel

# Install core (default components), then optional components required for a full bundle
cmake --install "${BUILD_DIR}"
cmake --install "${BUILD_DIR}" --component configuration
cmake --install "${BUILD_DIR}" --component colormaps
cmake --install "${BUILD_DIR}" --component dependencies

echo "Full macOS bundle installed to: ${INSTALL_DIR}"
echo "Bundle executable: ${INSTALL_DIR}/f3d.app/Contents/MacOS/f3d"

mv "$INSTALL_DIR"/f3d.app "$INSTALL_DIR"/F3D2.app
mv "$INSTALL_DIR"/F3D2.app "$INSTALL_DIR"/F3D.app

tar -C "$INSTALL_DIR" -cJf ./f3d_darwin_arm64.tar.xz ./F3D.app