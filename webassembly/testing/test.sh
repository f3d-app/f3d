#!/bin/bash

set -euo pipefail

DIR="$(cd "$(dirname "${BASH_SOURCE}")" && pwd)"

chromium \
    --disable-application-cache \
    --disable-extensions \
    --disable-notifications \
    --disable-restore-session-state \
    --new-window \
    --no-default-browser-check \
    --no-first-run \
    --enable-logging=stderr \
    --user-data-dir=$DIR/wasm_testing_out \
    --allow-file-access-from-files \
    file://"$DIR/test.html" 2> >(tee "$DIR/wasm_testing_out/stderr.log" >&2)

if grep -q "f3d_exit_code=0" "$DIR/wasm_testing_out/stderr.log"; then
  exit 0
fi

if grep -q "f3d_exit_code=1" "$DIR/wasm_testing_out/stderr.log"; then
  exit 1
fi

if grep -q "f3d_exit_code=2" "$DIR/wasm_testing_out/stderr.log"; then
  exit 2
fi
