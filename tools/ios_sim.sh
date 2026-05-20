#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build-ios-sim}"
CONFIG="${CONFIG:-Debug}"
TARGET="${TARGET:-BikeRaceEscapeAxmol}"
GENERATOR="${GENERATOR:-Xcode}"
TOOLCHAIN_FILE="${TOOLCHAIN_FILE:-${ROOT_DIR}/thirdparty/axmol/1k/ios.cmake}"

# Apple Silicon simulator arch is arm64; Intel is x86_64.
SIM_ARCH="${SIM_ARCH:-arm64}"

if [[ ! -f "${TOOLCHAIN_FILE}" ]]; then
  echo "error: TOOLCHAIN_FILE not found: ${TOOLCHAIN_FILE}" >&2
  echo "set TOOLCHAIN_FILE=/path/to/axmol/1k/ios.cmake" >&2
  exit 1
fi

# Ensure axmol source is writable from within the repo (CMake generates headers via configure_file).
# The build system reads `AX_ROOT` (not `_AX_ROOT`) and then prints it as `_AX_ROOT`.
export AX_ROOT="${ROOT_DIR}/thirdparty/axmol"

if [[ "${FORCE_CLEAN:-0}" == "1" ]]; then
  echo "[ios_sim] FORCE_CLEAN=1: removing ${BUILD_DIR}"
  rm -rf "${BUILD_DIR}"
fi

if [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  # If this build dir was previously configured for device, CMake will keep the wrong sysroot/libs.
  cache="${BUILD_DIR}/CMakeCache.txt"
  # If this build dir was previously configured for host/macOS (no iOS toolchain applied),
  # the ios toolchain won't override existing cache entries (it doesn't FORCE CMAKE_SYSTEM_NAME/ARCHS).
  if ! grep -qE '^CMAKE_SYSTEM_NAME:.*=iOS$' "${cache}" || \
     ! grep -qE '^CMAKE_XCODE_EFFECTIVE_PLATFORMS:.*-iphonesimulator' "${cache}"; then
    echo "[ios_sim] detected non-iOS cache in ${BUILD_DIR}, resetting cache files"
    rm -f "${BUILD_DIR}/CMakeCache.txt"
    rm -rf "${BUILD_DIR}/CMakeFiles"
  fi
  if [[ -f "${cache}" ]] && grep -qE 'CMAKE_XCODE_EFFECTIVE_PLATFORMS:.*-iphoneos' "${cache}"; then
    echo "[ios_sim] detected device cache in ${BUILD_DIR}, resetting cache files"
    rm -f "${BUILD_DIR}/CMakeCache.txt"
    rm -rf "${BUILD_DIR}/CMakeFiles"
  fi
fi

echo "[ios_sim] configure: ${BUILD_DIR}"
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G "${GENERATOR}" \
  -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
  -DCMAKE_C_COMPILER="$(xcrun --sdk iphonesimulator --find clang)" \
  -DCMAKE_CXX_COMPILER="$(xcrun --sdk iphonesimulator --find clang++)" \
  -DCMAKE_OBJC_COMPILER="$(xcrun --sdk iphonesimulator --find clang)" \
  -DCMAKE_OBJCXX_COMPILER="$(xcrun --sdk iphonesimulator --find clang++)" \
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
  -DThreads_FOUND=TRUE \
  -DCMAKE_HAVE_THREADS_LIBRARY=1 \
  -DCMAKE_USE_PTHREADS_INIT=1 \
  -DCMAKE_THREAD_LIBS_INIT="" \
  -DHAVE_PTHREAD_H=1 \
  -DHAVE_PTHREAD=1 \
  -DHAVE_UNISTD_H=1 \
  -DHAVE_FCNTL_H=1 \
  -DCMAKE_C_COMPILER_FORCED=1 \
  -DCMAKE_CXX_COMPILER_FORCED=1 \
  -DCMAKE_OBJC_COMPILER_FORCED=1 \
  -DCMAKE_OBJCXX_COMPILER_FORCED=1 \
  -DCMAKE_C_COMPILER_WORKS=1 \
  -DCMAKE_CXX_COMPILER_WORKS=1 \
  -DCMAKE_OBJCXX_COMPILER_WORKS=1 \
  -DARCHS="${SIM_ARCH}" \
  -DSIMULATOR=TRUE

# Generated projects may include '-latomic' (OpenAL) which doesn't exist on iOS.
if [[ -f "${BUILD_DIR}/BikeRaceEscapeAxmol.xcodeproj/project.pbxproj" ]]; then
  python3 "${ROOT_DIR}/tools/fix_xcode_pbxproj_remove_latomic.py" "${BUILD_DIR}/BikeRaceEscapeAxmol.xcodeproj/project.pbxproj" >/dev/null || true
fi

if [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  sysroot="$(grep -E '^CMAKE_OSX_SYSROOT:' "${BUILD_DIR}/CMakeCache.txt" | tail -1 || true)"
  effplats="$(grep -E '^CMAKE_XCODE_EFFECTIVE_PLATFORMS:' "${BUILD_DIR}/CMakeCache.txt" | tail -1 || true)"
  echo "[ios_sim] cache sysroot: ${sysroot:-"(missing)"}"
  echo "[ios_sim] cache plats:  ${effplats:-"(missing)"}"
fi

if [[ "${CONFIGURE_ONLY:-0}" == "1" ]]; then
  echo "[ios_sim] CONFIGURE_ONLY=1: skipping build"
  exit 0
fi

echo "[ios_sim] build: ${TARGET} (${CONFIG})"
cmake --build "${BUILD_DIR}" --config "${CONFIG}" --target "${TARGET}"
