#!/usr/bin/env bash
set -euo pipefail

# This script runs INSIDE the container after installation to /build/bundle
# It ensures VTK shared libraries are present and adjusts rpaths to be
# self-contained.

BUNDLE_DIR=${BUNDLE_DIR:-/build/bundle}
VTK_LIB_DIR=${VTK_LIB_DIR:-/opt/vtk/lib}

mkdir -p "${BUNDLE_DIR}/lib"

# 1) Copy VTK libs into bundle/lib
cp -a ${VTK_LIB_DIR}/*.so* "${BUNDLE_DIR}/lib/" || true

if command -v patchelf >/dev/null 2>&1; then
  set +e
  copy_deps() {
    local target="$1"
    for need in $(patchelf --print-needed "$target" 2>/dev/null); do
      [ -e "${BUNDLE_DIR}/lib/${need}" ] && continue
      if [ -e "${VTK_LIB_DIR}/${need}" ]; then
        cp -a "${VTK_LIB_DIR}/${need}" "${BUNDLE_DIR}/lib/"
        continue
      fi
      local base="${need%%.so*}"
      local cand
      cand=$(ls "${VTK_LIB_DIR}/${base}.so."* 2>/dev/null | head -n1)
      if [ -n "${cand:-}" ]; then
        cp -a "${cand}" "${BUNDLE_DIR}/lib/"
        ln -sf "$(basename "${cand}")" "${BUNDLE_DIR}/lib/${need}"
      fi
    done
  }

  # Set rpath and collect deps for the main binary
  if [ -f "${BUNDLE_DIR}/bin/f3d" ]; then
    patchelf --force-rpath --set-rpath '$ORIGIN/../lib' "${BUNDLE_DIR}/bin/f3d" || true
    copy_deps "${BUNDLE_DIR}/bin/f3d"
  fi

  # Set rpath and collect deps for our installed libs
  for so in "${BUNDLE_DIR}/lib/"*.so*; do
    [ -e "$so" ] || continue
    patchelf --force-rpath --set-rpath '$ORIGIN' "$so" || true
    copy_deps "$so"
  done
fi

echo "Bundle fixup complete in ${BUNDLE_DIR}"


