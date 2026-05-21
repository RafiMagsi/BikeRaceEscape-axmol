#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build-ios-device}"
CONFIG="${CONFIG:-Debug}"
TARGET="${TARGET:-BikeRaceEscapeAxmol}"
GENERATOR="${GENERATOR:-Xcode}"
TOOLCHAIN_FILE="${TOOLCHAIN_FILE:-${ROOT_DIR}/thirdparty/axmol/1k/ios.cmake}"
TEAM_ID="${TEAM_ID:-}"
DERIVED_DATA_PATH="${DERIVED_DATA_PATH:-${BUILD_DIR}/DerivedData}"

if [[ ! -f "${TOOLCHAIN_FILE}" ]]; then
  echo "error: TOOLCHAIN_FILE not found: ${TOOLCHAIN_FILE}" >&2
  echo "set TOOLCHAIN_FILE=/path/to/axmol/1k/ios.cmake" >&2
  exit 1
fi

# Ensure axmol source is writable from within the repo (CMake generates headers via configure_file).
# The build system reads `AX_ROOT` (not `_AX_ROOT`) and then prints it as `_AX_ROOT`.
export AX_ROOT="${ROOT_DIR}/thirdparty/axmol"

if [[ "${FORCE_CLEAN:-0}" == "1" ]]; then
  echo "[ios_device] FORCE_CLEAN=1: removing ${BUILD_DIR}"
  rm -rf "${BUILD_DIR}"
fi

already_pods=0
if [[ -d "${BUILD_DIR}/${TARGET}.xcworkspace" ]] && [[ -d "${BUILD_DIR}/Pods" ]] && [[ -f "${BUILD_DIR}/Podfile.lock" ]]; then
  already_pods=1
fi

if [[ "${already_pods}" == "1" ]] && [[ "${RECONFIGURE:-0}" != "1" ]]; then
  echo "[ios_device] Pods workspace detected; skipping CMake reconfigure (set RECONFIGURE=1 to force)"
else
if [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  # If this build dir was previously configured for simulator, CMake will keep the wrong sysroot/libs.
  cache="${BUILD_DIR}/CMakeCache.txt"
  # If this build dir was previously configured for host/macOS (no iOS toolchain applied),
  # the ios toolchain won't override existing cache entries (it doesn't FORCE CMAKE_SYSTEM_NAME/ARCHS).
  if ! grep -qE '^CMAKE_SYSTEM_NAME:.*=iOS$' "${cache}" || \
     ! grep -qE '^CMAKE_XCODE_EFFECTIVE_PLATFORMS:.*-iphoneos' "${cache}"; then
    echo "[ios_device] detected non-iOS cache in ${BUILD_DIR}, resetting cache files"
    rm -f "${BUILD_DIR}/CMakeCache.txt"
    rm -rf "${BUILD_DIR}/CMakeFiles"
  fi
  if [[ -f "${cache}" ]]; then
    if grep -qE 'CMAKE_XCODE_EFFECTIVE_PLATFORMS:.*-iphonesimulator' "${cache}" || \
       grep -qE '^CMAKE_OSX_SYSROOT:.*=iphonesimulator' "${cache}"; then
      echo "[ios_device] detected simulator cache in ${BUILD_DIR}, resetting cache files"
      rm -f "${BUILD_DIR}/CMakeCache.txt"
      rm -rf "${BUILD_DIR}/CMakeFiles"
    fi
  fi
fi

echo "[ios_device] configure: ${BUILD_DIR}"
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G "${GENERATOR}" \
  -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
  -DCMAKE_OSX_SYSROOT=iphoneos \
  -DCMAKE_C_COMPILER="$(xcrun --sdk iphoneos --find clang)" \
  -DCMAKE_CXX_COMPILER="$(xcrun --sdk iphoneos --find clang++)" \
  -DCMAKE_OBJC_COMPILER="$(xcrun --sdk iphoneos --find clang)" \
  -DCMAKE_OBJCXX_COMPILER="$(xcrun --sdk iphoneos --find clang++)" \
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
  -DARCHS=arm64 \
  -DSIMULATOR=FALSE \
  ${TEAM_ID:+-DPZ_DEVELOPMENT_TEAM=${TEAM_ID}}

# Generated projects may include '-latomic' (OpenAL) which doesn't exist on iOS.
if [[ -f "${BUILD_DIR}/BikeRaceEscapeAxmol.xcodeproj/project.pbxproj" ]]; then
  python3 "${ROOT_DIR}/tools/fix_xcode_pbxproj_remove_latomic.py" "${BUILD_DIR}/BikeRaceEscapeAxmol.xcodeproj/project.pbxproj" >/dev/null || true
fi

if [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  sysroot="$(grep -E '^CMAKE_OSX_SYSROOT:' "${BUILD_DIR}/CMakeCache.txt" | tail -1 || true)"
  effplats="$(grep -E '^CMAKE_XCODE_EFFECTIVE_PLATFORMS:' "${BUILD_DIR}/CMakeCache.txt" | tail -1 || true)"
  echo "[ios_device] cache sysroot: ${sysroot:-"(missing)"}"
  echo "[ios_device] cache plats:  ${effplats:-"(missing)"}"
fi

if [[ "${CONFIGURE_ONLY:-0}" == "1" ]]; then
  echo "[ios_device] CONFIGURE_ONLY=1: skipping build"
  exit 0
fi
fi # end skip-reconfigure guard

if [[ -d "${BUILD_DIR}/${TARGET}.xcworkspace" ]] && [[ -d "${BUILD_DIR}/Pods" ]]; then
  echo "[ios_device] build (workspace): ${TARGET} (${CONFIG})"
  xcodebuild -workspace "${BUILD_DIR}/${TARGET}.xcworkspace" \
    -scheme "${TARGET}" \
    -configuration "${CONFIG}" \
    -sdk iphoneos \
    -destination "generic/platform=iOS" \
    -derivedDataPath "${DERIVED_DATA_PATH}" \
    ${TEAM_ID:+DEVELOPMENT_TEAM=${TEAM_ID}} \
    build

  # Sanity check: if Swift-based pods (AdMob) are linked, the app executable will
  # reference @rpath/libswiftCore.dylib. Device installs will fail at launch if
  # the Swift stdlibs were not embedded into the built app bundle.
  #
  # Use our explicit DerivedData path to avoid build.db locks when Xcode is also open.
  app_path="${DERIVED_DATA_PATH}/Build/Products/${CONFIG}-iphoneos/${TARGET}.app"
  if [[ -f "${app_path}/${TARGET}" ]]; then
    if otool -L "${app_path}/${TARGET}" 2>/dev/null | grep -q '@rpath/libswiftCore.dylib'; then
      if [[ ! -f "${app_path}/Frameworks/libswiftCore.dylib" ]]; then
        echo "" >&2
        echo "error: device build references Swift runtime but Swift stdlibs were NOT embedded:" >&2
        echo "  missing: ${app_path}/Frameworks/libswiftCore.dylib" >&2
        echo "" >&2
        echo "Fix checklist:" >&2
        echo "  1) Run: (cd ${BUILD_DIR} && pod install)" >&2
        echo "  2) Re-run: TEAM_ID=... tools/ios_device.sh (without CONFIGURE_ONLY=1)" >&2
        echo "  3) If you also built in Xcode, stop the build and Close Xcode (build.db locks)," >&2
        echo "     then rerun this script (it uses: ${DERIVED_DATA_PATH})." >&2
        exit 2
      fi
    fi
  fi
else
  echo "[ios_device] build (cmake): ${TARGET} (${CONFIG})"
  cmake --build "${BUILD_DIR}" --config "${CONFIG}" --target "${TARGET}"
fi
