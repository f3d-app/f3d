#!/usr/bin/env bash

# cd to the root directory of the project
pushd "$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)/.."

set -euo pipefail

# Usage: scripts/build.sh [Release|Debug]
BUILD_TYPE=${1:-Release}

ROOT_DIR=$(pwd)
BUILD_DIR="${ROOT_DIR}/build_macos"
ARCH=$(uname -m)
# Allow version to be inferred from CMake after configure; fallback to 3.3.0
FALLBACK_VERSION="3.3.0"

# Clean tmp build dir each run; keep final bundle dir stable
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# Configure with macOS bundle and shared libs
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DF3D_MACOS_BUNDLE=ON

cd "$BUILD_DIR"
make -j9

mv "$BUILD_DIR"/bin/f3d.app "$BUILD_DIR"/bin/F3D2.app
mv "$BUILD_DIR"/bin/F3D2.app "$BUILD_DIR"/bin/F3D.app

# tar -C "$BUILD_DIR"/bin/ -cJf ./f3d_darwin_arm64.tar.xz ./F3D.app