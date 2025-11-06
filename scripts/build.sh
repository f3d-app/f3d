#!/usr/bin/env bash

# cd to the root directory of the project
pushd "$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)/.."

set -euo pipefail

rm -Rf ./build/
mkdir -p ./build/

cd ./build/

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DF3D_MACOS_BUNDLE=ON

make -j
