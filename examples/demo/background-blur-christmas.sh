#!/bin/bash

if [ "$#" -lt 1 ]; then
    printf 'Usage: %s <f3d-executable> [f3d-options...]\n' "$0" >&2
    exit 1
fi

f3d_executable="$1"
shift

"$f3d_executable" \
    ./examples/demo/meshes/christmas_low-poly_scene/scene.gltf \
    --final-shader "$(cat ./examples/shaders/background-blur.glsl)" \
    "$@"
