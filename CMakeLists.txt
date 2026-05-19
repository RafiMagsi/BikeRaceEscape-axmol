cmake_minimum_required(VERSION 3.22...4.1)

set(APP_NAME BikeRaceEscapeAxmol)

project(${APP_NAME} LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project-specific iOS bundle id (used by Xcode generator / Info.plist).
set(PZ_BUNDLE_ID "com.nextfiction.dinobikeescape" CACHE STRING "Bundle identifier (iOS/macOS)")

if(XCODE)
  set(CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY TRUE)
endif()

set(_is_axmol_embed FALSE)
set(_AX_USE_PREBUILT FALSE)

set(CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
)

# NOTE: The order of the cmake module "include(AXGame...)" statements matters
include(AXGameEngineOptions)
include(AXGameEngineSetup)

# Common cross-platform source files
file(GLOB_RECURSE GAME_HEADER CONFIGURE_DEPENDS
  Source/*.h Source/*.hpp
)

file(GLOB_RECURSE GAME_SOURCE CONFIGURE_DEPENDS
  Source/*.cpp Source/*.c Source/*.cc Source/*.mm Source/*.m
)

# Exclude legacy commercial/native bridge from core build
list(FILTER GAME_SOURCE EXCLUDE REGEX ".*/StoreBridge/.*")

file(GLOB_RECURSE LEGACY_BOX2D_SOURCES CONFIGURE_DEPENDS
  thirdparty/Box2D/*.cpp
)

list(APPEND GAME_SOURCE ${LEGACY_BOX2D_SOURCES})

set(GAME_INC_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/Source"
  "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty"
)

set(content_folder
  "${CMAKE_CURRENT_SOURCE_DIR}/Content"
)

if(APPLE)
  ax_mark_multi_resources(common_content_files RES_TO "Resources" FOLDERS ${content_folder})
elseif(WINDOWS)
  ax_mark_multi_resources(common_content_files RES_TO "Content" FOLDERS ${content_folder})
endif()

include(AXGameSourceSetup)

set(APP_SOURCES
  ${GAME_HEADER}
  ${GAME_SOURCE}
  ${common_content_files}
)

include(AXGameTargetSetup)

# Mark app resources / IDE props
ax_setup_app_config(${APP_NAME})

# Platform defaults (patched for this repo in cmake/modules/AXGamePlatformSetup.cmake)
include(AXGamePlatformSetup)

# ---- Project-specific iOS runtime fixes ----

# ---- App icons (iOS + Android) ----
option(PZ_GENERATE_APP_ICONS "Generate iOS/Android app icons from art/icon-source.png during build" ON)
set(PZ_APP_ICON_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/art/icon-source.png" CACHE FILEPATH "Source PNG for app icons")

if(PZ_GENERATE_APP_ICONS AND EXISTS "${PZ_APP_ICON_SOURCE}")
  find_package(Python3 COMPONENTS Interpreter)
  if(NOT Python3_Interpreter_FOUND)
    message(WARNING "PZ_GENERATE_APP_ICONS is ON but Python3 interpreter wasn't found; skipping icon generation")
    set(PZ_GENERATE_APP_ICONS OFF)
  else()
    execute_process(
      COMMAND "${Python3_EXECUTABLE}" -c "import PIL"
      RESULT_VARIABLE _pz_pil_ok
      OUTPUT_QUIET
      ERROR_QUIET
    )
    if(NOT _pz_pil_ok EQUAL 0)
      message(WARNING "PZ_GENERATE_APP_ICONS is ON but Python 'Pillow' (PIL) is missing. Install with: python3 -m pip install pillow. Skipping icon generation.")
      set(PZ_GENERATE_APP_ICONS OFF)
    endif()
  endif()
endif()

if(PZ_GENERATE_APP_ICONS)
  set(_PZ_ICON_GEN_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_app_icons.py")
  set(_PZ_IOS_APPICONSET "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/ios/targets/ios/Images.xcassets/AppIcon.appiconset")
  set(_PZ_ANDROID_RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proj.android/app/src/main/res")
  set(_PZ_ICON_STAMP "${CMAKE_BINARY_DIR}/pz_generated_icons.stamp")

  if(EXISTS "${_PZ_ICON_GEN_SCRIPT}")
    add_custom_command(
      OUTPUT "${_PZ_ICON_STAMP}"
      COMMAND "${Python3_EXECUTABLE}" "${_PZ_ICON_GEN_SCRIPT}"
              --input "${PZ_APP_ICON_SOURCE}"
              --ios-out "${_PZ_IOS_APPICONSET}"
              --android-out "${_PZ_ANDROID_RES_DIR}"
      COMMAND "${CMAKE_COMMAND}" -E touch "${_PZ_ICON_STAMP}"
      DEPENDS "${PZ_APP_ICON_SOURCE}" "${_PZ_ICON_GEN_SCRIPT}"
      COMMENT "Generating iOS/Android app icons from ${PZ_APP_ICON_SOURCE}"
      VERBATIM
    )
    add_custom_target(pz_generate_app_icons DEPENDS "${_PZ_ICON_STAMP}")
    add_dependencies(${APP_NAME} pz_generate_app_icons)
  else()
    message(WARNING "PZ_GENERATE_APP_ICONS is ON but tools/generate_app_icons.py not found; skipping icon generation")
  endif()
endif()

if(IOS AND XCODE)
  # Optional: set your Apple Developer Team ID (e.g. "ABCDE12345") to enable device code signing
  # for the generated Xcode project.
  set(PZ_DEVELOPMENT_TEAM "" CACHE STRING "Apple Developer Team ID for iOS code signing (Xcode DEVELOPMENT_TEAM)")

  # Ensure Xcode compiles the asset catalog (AppIcon) into the bundle.
  set(_PZ_IOS_ASSETS_XCASSETS "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/ios/targets/ios/Images.xcassets")
  if(EXISTS "${_PZ_IOS_ASSETS_XCASSETS}")
    target_sources(${APP_NAME} PRIVATE "${_PZ_IOS_ASSETS_XCASSETS}")
    set_source_files_properties("${_PZ_IOS_ASSETS_XCASSETS}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(${APP_NAME} PROPERTIES
      XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
    )
  endif()

  # Xcode 15+ may sandbox user scripts; our generated build phases write into the build tree.
  set_target_properties(${APP_NAME} PROPERTIES
    XCODE_ATTRIBUTE_ENABLE_USER_SCRIPT_SANDBOXING "NO"
  )

  if(PZ_DEVELOPMENT_TEAM)
    set_target_properties(${APP_NAME} PROPERTIES
      XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${PZ_DEVELOPMENT_TEAM}"
    )
  endif()

  # Use the template Info.plist which already declares CFBundleIconName=AppIcon.
  set_target_properties(${APP_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/ios/targets/ios/Info.plist"
  )

  # Game Center (GameKit)
  # Note: AXGameTargetSetup.cmake uses the plain `target_link_libraries` signature, so keep it consistent.
  target_link_libraries(${APP_NAME} "-framework GameKit")

  # ---- iOS AdMob (Google Mobile Ads) optional integration ----
  # To enable real AdMob ads on iOS, download GoogleMobileAds.xcframework and place it at:
  #   thirdparty/GoogleMobileAds/GoogleMobileAds.xcframework
  # This keeps the repo buildable without the SDK.
  if(IOS)
    set(_PZ_GMA_XCFW "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/GoogleMobileAds/GoogleMobileAds.xcframework")
    if(CMAKE_OSX_SYSROOT MATCHES ".*simulator.*")
      set(_PZ_GMA_SLICE "ios-arm64_x86_64-simulator")
    else()
      set(_PZ_GMA_SLICE "ios-arm64")
    endif()
    set(_PZ_GMA_SLICE_DIR "${_PZ_GMA_XCFW}/${_PZ_GMA_SLICE}")
    set(_PZ_GMA_FRAMEWORK "${_PZ_GMA_SLICE_DIR}/GoogleMobileAds.framework")
    if(EXISTS "${_PZ_GMA_FRAMEWORK}")
      set_target_properties(${APP_NAME} PROPERTIES
        XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS "${_PZ_GMA_SLICE_DIR} $(inherited)"
      )
      target_link_libraries(${APP_NAME} "-framework GoogleMobileAds")

      # Embed (and re-sign on device) so the app can run outside Xcode too.
      add_custom_command(TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${_PZ_GMA_FRAMEWORK}" "$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks/GoogleMobileAds.framework"
        COMMAND /bin/sh -c "if [ -n \"$EXPANDED_CODE_SIGN_IDENTITY\" ] && [ -d \"$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks/GoogleMobileAds.framework\" ]; then /usr/bin/codesign --force --sign \"$EXPANDED_CODE_SIGN_IDENTITY\" --timestamp=none --preserve-metadata=identifier,entitlements \"$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks/GoogleMobileAds.framework\"; else :; fi"
        VERBATIM
      )
    else()
      message(STATUS "GoogleMobileAds.xcframework not found at ${_PZ_GMA_XCFW}; AdMob provider will run in stub mode.")
    endif()
  endif()

  # Ensure dSYM is generated for archives (required for symbol upload / crash symbolication).
  set_target_properties(${APP_NAME} PROPERTIES
    XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
    XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES"
  )
endif()

# Axmol's iOS OpenAL build outputs `soft_oal.framework` under `${CMAKE_BINARY_DIR}/build/<cfg>-<sdk>/`,
# but the app link line may expect it under `${CMAKE_BINARY_DIR}/axmol/engine/3rdparty/openal/<cfg>-<sdk>/`.
if(IOS)
  if(CMAKE_OSX_SYSROOT MATCHES ".*simulator.*")
    set(_PZ_IOS_SDK_SUFFIX "iphonesimulator")
  else()
    set(_PZ_IOS_SDK_SUFFIX "iphoneos")
  endif()

  set(_PZ_SOFT_OAL_SRC "${CMAKE_BINARY_DIR}/build/$<CONFIG>-${_PZ_IOS_SDK_SUFFIX}/soft_oal.framework")
  set(_PZ_SOFT_OAL_DST "${CMAKE_BINARY_DIR}/axmol/engine/3rdparty/openal/$<CONFIG>-${_PZ_IOS_SDK_SUFFIX}/soft_oal.framework")

  add_custom_target(pz_fix_soft_oal_framework
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/axmol/engine/3rdparty/openal/$<CONFIG>-${_PZ_IOS_SDK_SUFFIX}"
    # During Archive (and sometimes normal Xcode builds), OpenAL's `soft_oal.framework` is produced
    # in Xcode's DerivedData (BUILT_PRODUCTS_DIR), not under `${CMAKE_BINARY_DIR}/build/<cfg>-<sdk>/`.
    # Prefer DerivedData if present; fall back to the legacy location for local builds.
    COMMAND /bin/sh -c "set -e; src=\"$BUILT_PRODUCTS_DIR/soft_oal.framework\"; if [ -n \"$BUILT_PRODUCTS_DIR\" ] && [ -d \"$src\" ]; then :; elif [ -d \"${_PZ_SOFT_OAL_SRC}\" ]; then src=\"${_PZ_SOFT_OAL_SRC}\"; else echo \"warning: soft_oal.framework not found (DerivedData or ${_PZ_SOFT_OAL_SRC}); skipping pz_fix_soft_oal_framework\"; exit 0; fi; \"${CMAKE_COMMAND}\" -E copy_directory \"$src\" \"${_PZ_SOFT_OAL_DST}\""
    VERBATIM
  )

  if(TARGET OpenAL)
    add_dependencies(pz_fix_soft_oal_framework OpenAL)
  endif()
  add_dependencies(${APP_NAME} pz_fix_soft_oal_framework)

  # Simulator runtime fix: ensure dyld can load OpenAL at launch.
  # IMPORTANT: do NOT copy the framework into the device .app here, because it would overwrite
  # Xcode's signed embedded framework and cause install errors (0xe800801c: No code signature found).
  if(CMAKE_OSX_SYSROOT MATCHES ".*simulator.*")
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks"
      COMMAND /bin/sh -c "set -e; src=\"$BUILT_PRODUCTS_DIR/soft_oal.framework\"; if [ -n \"$BUILT_PRODUCTS_DIR\" ] && [ -d \"$src\" ]; then :; elif [ -d \"${_PZ_SOFT_OAL_SRC}\" ]; then src=\"${_PZ_SOFT_OAL_SRC}\"; else echo \"warning: soft_oal.framework not found; skipping embed copy\"; exit 0; fi; \"${CMAKE_COMMAND}\" -E copy_directory \"$src\" \"$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks/soft_oal.framework\""
      VERBATIM
    )
  else()
    # Device: the CMake-generated Xcode project stages the .app via its own scripts, which won't
    # automatically embed/sign this framework. Copy it in, then codesign it with the same identity
    # Xcode uses for the app to avoid install failures.
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks"
      COMMAND /bin/sh -c "set -e; src=\"$BUILT_PRODUCTS_DIR/soft_oal.framework\"; if [ -n \"$BUILT_PRODUCTS_DIR\" ] && [ -d \"$src\" ]; then :; elif [ -d \"${_PZ_SOFT_OAL_SRC}\" ]; then src=\"${_PZ_SOFT_OAL_SRC}\"; else echo \"warning: soft_oal.framework not found; skipping embed copy\"; exit 0; fi; \"${CMAKE_COMMAND}\" -E copy_directory \"$src\" \"$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks/soft_oal.framework\""
      # Use Xcode-provided signing identity at build time (not configure time).
      COMMAND /bin/sh -c "if [ -n \"$EXPANDED_CODE_SIGN_IDENTITY\" ]; then /usr/bin/codesign --force --sign \"$EXPANDED_CODE_SIGN_IDENTITY\" --timestamp=none --preserve-metadata=identifier,entitlements \"$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks/soft_oal.framework\"; else echo \"warning: EXPANDED_CODE_SIGN_IDENTITY is empty; soft_oal.framework left unsigned\"; fi"
      VERBATIM
    )
  endif()
endif()

# Ship Axmol-generated runtime assets (compiled shaders) into the app bundle.
add_custom_command(TARGET ${APP_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_BINARY_DIR}/runtime
          $<TARGET_FILE_DIR:${APP_NAME}>/runtime
)

# Make sure the AXGameFinalSetup is included at the end of this file
include(AXGameFinalSetup)
