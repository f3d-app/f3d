#!/bin/bash

if [ "$#" -lt 1 ]; then
    printf 'Usage: %s <f3d-executable> [f3d-options...]\n' "$0" >&2
    exit 1
fi

f3d_executable="$1"
shift

"$f3d_executable" \
    ../saviem_sc_10_renault.glb \
    --final-shader "$(cat ./examples/shaders/comics.glsl)" \
    "$@"
