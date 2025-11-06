#!/usr/bin/env bash

# cd to the root directory of the project
pushd "$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)/.."

set -euo pipefail

# Usage: scripts/build.sh [Release|Debug]
BUILD_TYPE=${1:-Release}

ROOT_DIR=$(pwd)
BUILD_DIR="${ROOT_DIR}/build_macos/.build_tmp"
ARCH=$(uname -m)
# Allow version to be inferred from CMake after configure; fallback to 3.3.0
FALLBACK_VERSION="3.3.0"

# Staging directory for the full bundle
BUNDLE_BASE_DIR="${ROOT_DIR}/build_macos"
mkdir -p "${BUNDLE_BASE_DIR}"

# Clean tmp build dir each run; keep final bundle dir stable
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# Configure with macOS bundle and shared libs
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DF3D_MACOS_BUNDLE=ON \
  -DBUILD_SHARED_LIBS=ON \
  -DF3D_PLUGINS_STATIC_BUILD=ON \
  ${F3D_DEPENDENCIES_DIR:+-DF3D_DEPENDENCIES_DIR="${F3D_DEPENDENCIES_DIR}"}

# Extract version from CMake cache if available
if grep -q '^F3D_VERSION:STRING=' "${BUILD_DIR}/CMakeCache.txt" 2>/dev/null; then
  F3D_VERSION=$(sed -n 's/^F3D_VERSION:STRING=//p' "${BUILD_DIR}/CMakeCache.txt")
else
  F3D_VERSION="${FALLBACK_VERSION}"
fi

BUNDLE_NAME="F3D-${F3D_VERSION}-macOS-${ARCH}"
BUNDLE_DIR="${BUNDLE_BASE_DIR}/${BUNDLE_NAME}"

# Fresh bundle dir each run
rm -rf "${BUNDLE_DIR}"
mkdir -p "${BUNDLE_DIR}"

# Build
cmake --build "${BUILD_DIR}" --parallel

# Install components to produce a self-contained bundle
# - application: installs F3D.app at prefix root
# - library: installs libf3d and any shared libs built here
# - configuration: default configs into bundle Resources on macOS
# - colormaps: default colormaps into Resources
# - dependencies: copies runtime deps discovered by CMake (requires F3D_DEPENDENCIES_DIR for external deps)
cmake --install "${BUILD_DIR}" --prefix "${BUNDLE_DIR}" --component application
cmake --install "${BUILD_DIR}" --prefix "${BUNDLE_DIR}" --component library
cmake --install "${BUILD_DIR}" --prefix "${BUNDLE_DIR}" --component configuration
cmake --install "${BUILD_DIR}" --prefix "${BUNDLE_DIR}" --component colormaps
cmake --install "${BUILD_DIR}" --prefix "${BUNDLE_DIR}" --component dependencies

# macOS bundle fixup: move dylibs into the .app and rewrite install names/rpaths
APP_DIR="${BUNDLE_DIR}/F3D.app"
EXE_PATH="${APP_DIR}/Contents/MacOS/f3d"
FRAMEWORKS_DIR="${APP_DIR}/Contents/Frameworks"
mkdir -p "${FRAMEWORKS_DIR}"

# Collect and copy any dylibs staged under the prefix (commonly in lib/)
if [ -d "${BUNDLE_DIR}/lib" ]; then
  find "${BUNDLE_DIR}/lib" -maxdepth 1 -type f -name "*.dylib" -exec cp -a {} "${FRAMEWORKS_DIR}/" \;
fi
# Also collect any stray dylibs directly under the prefix root
find "${BUNDLE_DIR}" -maxdepth 1 -type f -name "*.dylib" -exec cp -a {} "${FRAMEWORKS_DIR}/" \; 2>/dev/null || true

# Helper to rewrite dependencies to @rpath for items available in Frameworks
rewrite_deps() {
  local target="$1"
  local dep
  while IFS= read -r dep; do
    [ -z "${dep}" ] && continue
    local base
    base=$(basename "${dep}")
    if [ -f "${FRAMEWORKS_DIR}/${base}" ]; then
      install_name_tool -change "${dep}" "@rpath/${base}" "${target}" || true
    fi
  done < <(otool -L "${target}" | tail -n +2 | awk '{print $1}')
}

if [ -f "${EXE_PATH}" ]; then
  # Ensure the executable can search libs inside the bundle
  install_name_tool -delete_rpath "@loader_path/../../../lib" "${EXE_PATH}" 2>/dev/null || true
  install_name_tool -add_rpath "@executable_path/../Frameworks" "${EXE_PATH}" 2>/dev/null || true
  rewrite_deps "${EXE_PATH}"
fi

# For each dylib in Frameworks: set id to @rpath/<name> and rewrite intra-bundle deps
find "${FRAMEWORKS_DIR}" -type f -name "*.dylib" | while read -r dylib; do
  base=$(basename "${dylib}")
  install_name_tool -id "@rpath/${base}" "${dylib}" 2>/dev/null || true
  # Let each dylib search its neighbors using @loader_path (common pattern)
  install_name_tool -add_rpath "@loader_path" "${dylib}" 2>/dev/null || true
  rewrite_deps "${dylib}"
done

# Optional: remove the external lib directory to avoid confusion
if [ -d "${BUNDLE_DIR}/lib" ]; then
  rmdir "${BUNDLE_DIR}/lib" 2>/dev/null || true
fi

mv "${BUNDLE_DIR}/f3d.app" "${BUNDLE_DIR}/F3D2.app"
mv "${BUNDLE_DIR}/F3D2.app" "${BUNDLE_DIR}/F3D.app"

# Final output hint
echo "Full macOS bundle staged at: ${BUNDLE_DIR}"
echo "- App: ${BUNDLE_DIR}/F3D.app"
echo "- Libs inside app: ${FRAMEWORKS_DIR}"

tar -C "$BUNDLE_DIR" -cJf ./f3d_darwin_arm64.tar.xz ./F3D.app