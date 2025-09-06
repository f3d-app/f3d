#!/bin/bash

set -euo pipefail

cmake -S /src -B /src/_wasm_build \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_BUILD_TYPE=$1 \
    -DCMAKE_FIND_ROOT_PATH:PATH=/depends \
    -DF3D_MODULE_UI=OFF \
    -DF3D_PLUGIN_BUILD_ASSIMP=ON \
    -DF3D_PLUGIN_BUILD_DRACO=ON \
    -DF3D_PLUGIN_BUILD_HDF=OFF \
    -DF3D_PLUGIN_BUILD_OCCT=ON \
    -DF3D_STRICT_BUILD=ON

cmake --build /src/_wasm_build

# Copy generated js/wasm to dist
mkdir -p /src/dist
mv /src/_wasm_build/bin/* /src/dist
