#!/usr/bin/env bash
set -euo pipefail

# Build F3D on Linux using a local Ubuntu container image from macOS (or anywhere with Docker/Podman).
#
# Usage:
#   scripts/build_linux.sh [build-type]
#
# Env (optional):
#   F3D_DOCKER_CLIENT   Docker client command (docker or podman). Default: docker
#   F3D_DOCKER_PLATFORM Docker platform (e.g., linux/amd64). Default: linux/amd64
#   CMAKE_BUILD_PARALLEL_LEVEL  Build parallelism inside the container
#
# Outputs:
#   - Final bundle in ./build_linux/F3D-3.3.0-Linux-x86_64 (no extra _build directory)

BUILD_TYPE=${1:-Release}
DOCKER_CLIENT=${F3D_DOCKER_CLIENT:-docker}
# Default to x86_64 builds unless overridden
TARGET_PLATFORM=${F3D_DOCKER_PLATFORM:-linux/amd64}
TARGET_ARCH=${TARGET_PLATFORM#linux/}
LOCAL_IMAGE_TAG="f3d-linux-${TARGET_ARCH}"

ROOT_DIR=$(cd "$(dirname "$0")"/.. && pwd)
BUILD_DIR="${ROOT_DIR}/build_linux"
BUILD_SUBDIR="${BUILD_DIR}/.build_tmp"
BUNDLE_NAME="F3D-3.3.0-Linux-x86_64"
BUNDLE_DIR="${BUILD_DIR}/${BUNDLE_NAME}"

mkdir -p "${BUILD_DIR}"

# Always pass the platform (defaults to linux/amd64)
platform_args="--platform ${TARGET_PLATFORM}"

# Build the local image if not present
if ! ${DOCKER_CLIENT} image inspect "${LOCAL_IMAGE_TAG}" >/dev/null 2>&1; then
  ${DOCKER_CLIENT} build ${platform_args} -t "${LOCAL_IMAGE_TAG}" -f "${ROOT_DIR}/Dockerfile.linux" "${ROOT_DIR}"
fi

# Run as the current user to avoid root-owned outputs
uid_gid=("--user" "$(id -u):$(id -g)")

${DOCKER_CLIENT} run --rm \
  ${platform_args} \
  "${uid_gid[@]}" \
  -e CMAKE_BUILD_PARALLEL_LEVEL \
  -v "${ROOT_DIR}":/src \
  -v "${BUILD_DIR}":/build \
  -w /build \
  "${LOCAL_IMAGE_TAG}" \
  bash -lc "\
    rm -rf /build/.build_tmp /build/${BUNDLE_NAME} \
    && cmake -S /src -B /build/.build_tmp \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DF3D_LINUX_GENERATE_MAN=OFF \
      -DVTK_DIR=/opt/vtk/lib/cmake/vtk-9.5 \
      -DF3D_DEPENDENCIES_DIR=/opt/vtk/lib \
      -DCMAKE_INSTALL_PREFIX=/build/${BUNDLE_NAME} \
      -DCMAKE_INSTALL_RPATH='\$ORIGIN/../lib' \
      -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON \
    && cmake --build /build/.build_tmp --parallel \
    && cmake --install /build/.build_tmp --component application \
    && cmake --install /build/.build_tmp --component library \
    && cmake --install /build/.build_tmp --component configuration \
    && cmake --install /build/.build_tmp --component dependencies \
    && rm -rf /build/.build_tmp
  "

echo "Linux bundle installed to: $BUILD_DIR"
tar -C "$BUILD_DIR" -cJf ./f3d_linux_x86_64.tar.xz ./F3D-3.3.0-Linux-x86_64
