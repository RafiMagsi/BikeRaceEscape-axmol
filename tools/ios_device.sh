#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build-ios-device}"
CONFIG="${CONFIG:-Debug}"
TARGET="${TARGET:-BikeRaceEscapeAxmol}"
GENERATOR="${GENERATOR:-Xcode}"
TOOLCHAIN_FILE="${TOOLCHAIN_FILE:-/Users/rafi/development/axmol/1k/ios.cmake}"

if [[ ! -f "${TOOLCHAIN_FILE}" ]]; then
  echo "error: TOOLCHAIN_FILE not found: ${TOOLCHAIN_FILE}" >&2
  echo "set TOOLCHAIN_FILE=/path/to/axmol/1k/ios.cmake" >&2
  exit 1
fi

if [[ "${FORCE_CLEAN:-0}" == "1" ]]; then
  echo "[ios_device] FORCE_CLEAN=1: removing ${BUILD_DIR}"
  rm -rf "${BUILD_DIR}"
fi

if [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  # If this build dir was previously configured for simulator, CMake will keep the wrong sysroot/libs.
  cache="${BUILD_DIR}/CMakeCache.txt"
  if grep -qE 'CMAKE_XCODE_EFFECTIVE_PLATFORMS:.*-iphonesimulator' "${cache}" || \
     grep -qE '^CMAKE_OSX_SYSROOT:.*=iphonesimulator' "${cache}"; then
    echo "[ios_device] detected simulator cache in ${BUILD_DIR}, resetting cache files"
    rm -f "${BUILD_DIR}/CMakeCache.txt"
    rm -rf "${BUILD_DIR}/CMakeFiles"
  fi
fi

echo "[ios_device] configure: ${BUILD_DIR}"
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G "${GENERATOR}" \
  -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
  -DARCHS=arm64 \
  -DSIMULATOR=FALSE

if [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  sysroot="$(grep -E '^CMAKE_OSX_SYSROOT:' "${BUILD_DIR}/CMakeCache.txt" | tail -1 || true)"
  effplats="$(grep -E '^CMAKE_XCODE_EFFECTIVE_PLATFORMS:' "${BUILD_DIR}/CMakeCache.txt" | tail -1 || true)"
  echo "[ios_device] cache sysroot: ${sysroot:-"(missing)"}"
  echo "[ios_device] cache plats:  ${effplats:-"(missing)"}"
fi

echo "[ios_device] build: ${TARGET} (${CONFIG})"
cmake --build "${BUILD_DIR}" --config "${CONFIG}" --target "${TARGET}"
