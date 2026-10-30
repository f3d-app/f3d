#!/bin/bash

set -euo pipefail

export LD_LIBRARY_PATH="/src/_wasm_build/lib:/depends/lib"
export LDFLAGS="-L/src/_wasm_build/lib -L/depends/lib"

cmake -S /src -B /src/_wasm_build \
    -DBUILD_SHARED_LIBS=ON \
    -DF3D_PLUGINS_STATIC_BUILD=OFF \
    -DCMAKE_BUILD_TYPE=$1 \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_FIND_ROOT_PATH:PATH=/depends \
    -DCMAKE_MODULE_PATH=/depends/lib/cmake/OpenVDB/ \
    -DF3D_ENABLE_SOVERSION=OFF \
    -DF3D_MODULE_UI=OFF \
    -DF3D_MODULE_WEBP=ON \
    -DF3D_PLUGIN_BUILD_ALEMBIC=ON \
    -DF3D_PLUGIN_BUILD_ASSIMP=ON \
    -DF3D_PLUGIN_BUILD_DRACO=ON \
    -DF3D_PLUGIN_BUILD_HDF=ON \
    -DF3D_PLUGIN_BUILD_OCCT=ON \
    -DF3D_PLUGIN_BUILD_PDAL=OFF \
    -DF3D_PLUGIN_BUILD_USD=ON \
    -DF3D_PLUGIN_BUILD_VDB=ON \
    -DF3D_PLUGIN_BUILD_WEBIFC=ON \
    -DF3D_STRICT_BUILD=ON

cmake --build /src/_wasm_build

# Copy generated js/wasm to dist
mkdir -p /src/dist
mv /src/_wasm_build/bin/* /src/dist
mv /src/_wasm_build/lib/*.so /src/dist
mv /depends/lib/*.so /src/dist
