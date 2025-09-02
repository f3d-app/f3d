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
    --v=INFO:CONSOLE \
    --user-data-dir=$DIR/wasm_testing_out \
    --enable-features=WebAssemblyExperimentalJSPI \
    --allow-file-access-from-files \
    file://"$DIR/test.html"


