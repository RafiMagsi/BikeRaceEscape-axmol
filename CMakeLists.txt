cmake_minimum_required(VERSION 3.22...4.1)

set(APP_NAME BikeRaceEscapeAxmol)

project(${APP_NAME} LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# In our iOS Xcode-generator builds the compiler ID is often "unknown" during CMake configure,
# which can prevent CMake from emitting the correct per-target `CLANG_CXX_LANGUAGE_STANDARD`
# settings into the generated `.xcodeproj`. Hard-force the standard via compile flags on Apple.
if(APPLE)
  add_compile_options(
    "$<$<COMPILE_LANGUAGE:CXX>:-std=gnu++20>"
  )
endif()

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

# Ensure Box2D sources compile with correct include paths
set_source_files_properties(${LEGACY_BOX2D_SOURCES} PROPERTIES
  COMPILE_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR}/thirdparty"
)

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

# iOS/Simulator: FreeType's `builds/unix/ftsystem.c` gates `<unistd.h>`/`<fcntl.h>`
# behind `HAVE_UNISTD_H` / `HAVE_FCNTL_H`. Our sandboxed CMake configure can fail
# header probes, leaving these undefined and breaking the build.
#
# On Apple platforms these headers are always available, so define them explicitly.
if(IOS AND TARGET freetype)
  target_compile_definitions(freetype PRIVATE
    HAVE_UNISTD_H
    HAVE_FCNTL_H
  )
endif()

# iOS Simulator: some toolchain/probing paths end up compiling 3rdparty code
# without our top-level C++ standard. Clipper2 requires C++17 for `std::optional`
# and type traits helpers.
if(IOS)
  foreach(_PZ_CXX17_TARGET clipper2 alcommon OpenAL)
    if(TARGET ${_PZ_CXX17_TARGET})
      set_target_properties(${_PZ_CXX17_TARGET} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
      )
      # In our sandboxed/toolchain-driven iOS builds CMake often can't detect the compiler ID,
      # so Xcode projects may not get `CLANG_CXX_LANGUAGE_STANDARD` set. Force the standard
      # explicitly via flags for these targets.
      target_compile_options(${_PZ_CXX17_TARGET} PRIVATE "-std=gnu++17")
    endif()
  endforeach()
endif()

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
  option(PZ_USE_COCOAPODS "Enable CocoaPods integration (iOS)" ON)

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

  # Ensure this target is treated as iOS-only.
  # If Xcode thinks the app can run "Designed for iPhone/iPad" on Mac, it may show "My Mac"
  # as a run destination and try to sign for macOS, which is not what we want here.
  set_target_properties(${APP_NAME} PROPERTIES
    XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "iphoneos iphonesimulator"
    XCODE_ATTRIBUTE_SUPPORTS_MACCATALYST "NO"
    XCODE_ATTRIBUTE_SUPPORTS_MAC_DESIGNED_FOR_IPHONE_IPAD "NO"
  )

  if(PZ_DEVELOPMENT_TEAM)
    set_target_properties(${APP_NAME} PROPERTIES
      XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${PZ_DEVELOPMENT_TEAM}"
    )
  endif()

  # Use the template Info.plist which already declares CFBundleIconName=AppIcon.
  set_target_properties(${APP_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/ios/targets/ios/Info.plist"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/ios/targets/ios/BikeRaceEscapeAxmol.entitlements"
  )

  # Game Center (GameKit)
  # Note: AXGameTargetSetup.cmake uses the plain `target_link_libraries` signature, so keep it consistent.
  target_link_libraries(${APP_NAME} "-framework GameKit")

  # ---- CocoaPods Setup for iOS ----
  # Create Podfile for CocoaPods dependency management (pods installed via build script)
  if(IOS AND PZ_USE_COCOAPODS)
    set(_PODFILE_PATH "${CMAKE_BINARY_DIR}/Podfile")
    set(_PODS_DIR "${CMAKE_BINARY_DIR}/Pods")

    # Create Podfile if it doesn't exist
    if(NOT EXISTS "${_PODFILE_PATH}")
      file(WRITE "${_PODFILE_PATH}" "platform :ios, '12.0'\n\ntarget '${APP_NAME}' do\n  pod 'Google-Mobile-Ads-SDK', '~> 13.4.0'\n  pod 'ChartboostSDK'\nend\n")
      message(STATUS "[CocoaPods] Created Podfile at ${_PODFILE_PATH}")
      message(STATUS "[CocoaPods] Run 'pod install' in ${CMAKE_BINARY_DIR} to install dependencies")
    else()
      message(STATUS "[CocoaPods] Using existing Podfile at ${_PODFILE_PATH}")
    endif()

    # Pods framework configuration
    #
    # Do NOT touch Xcode search paths here.
    # - CocoaPods wires everything via the generated `.xcworkspace` + `Pods-*.xcconfig`.
    # - Setting `XCODE_ATTRIBUTE_HEADER_SEARCH_PATHS` / `FRAMEWORK_SEARCH_PATHS` from CMake
    #   can overwrite Axmol's include paths, leading to errors like `"axmol.h" file not found`.
    # CocoaPods builds its aggregate library (`libPods-<Target>.a`) into Xcode's default
    # BUILT_PRODUCTS_DIR: `$(BUILD_DIR)/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)`.
    #
    # Axmol/CMake templates often override CONFIGURATION_BUILD_DIR to a custom `bin/...` location,
    # which makes the linker look for `libPods-${APP_NAME}.a` in the wrong directory.
    #
    # Fix: always use the default Xcode build products dir when CocoaPods are enabled.
    set_target_properties(${APP_NAME} PROPERTIES
      XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "$(BUILD_DIR)/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)"
    )

    if(EXISTS "${_PODS_DIR}")
      message(STATUS "[CocoaPods] Pods directory detected")
    endif()
    message(STATUS "[CocoaPods] Build from workspace: ${CMAKE_BINARY_DIR}/${APP_NAME}.xcworkspace")
  endif()

  # ---- Swift runtime (CocoaPods) ----
  # Let CocoaPods' generated xcconfig control Swift embedding flags.
  # Overriding `EMBEDDED_CONTENT_CONTAINS_SWIFT` here causes CocoaPods warnings and
  # can lead to inconsistent Swift stdlib embedding behavior.

  # Ensure Swift runtime can be resolved from the OS when present.
  # This avoids depending solely on `@executable_path/Frameworks` for `@rpath/libswift*.dylib`
  # and helps prevent "duplicate class" warnings when the OS already provides Swift stdlibs.
  #
  # Keep `@executable_path/Frameworks` as well (needed if Xcode decides to embed Swift libs).
  set_target_properties(${APP_NAME} PROPERTIES
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "$(inherited) @executable_path/Frameworks /usr/lib/swift"
  )

  # NOTE: We intentionally do not force-copy Swift stdlibs here.
  # With modern iOS versions, Swift stdlibs are in the OS (`/usr/lib/swift`).
  # We add `/usr/lib/swift` to runpaths (above) so `@rpath/libswift*.dylib` resolves
  # without embedding, avoiding duplicate-class warnings.

  # CocoaPods/Xcode handle Swift compatibility libs and embedding; avoid forcing link flags here.

  # NOTE: Avoid manually copying Swift dylibs post-build. It frequently breaks code signing on-device.

  # ---- iOS AdMob: inject `GADApplicationIdentifier` at build time ----
  # Google Mobile Ads expects `GADApplicationIdentifier` in the *built* app's Info.plist.
  # We keep it out of the repo template plist and pull it from `Content/ads.plist` instead.
  if(IOS)
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
      COMMAND /bin/sh -c "set -e; ads_plist=\"${CMAKE_CURRENT_SOURCE_DIR}/Content/ads.plist\"; if [ ! -f \"$ads_plist\" ]; then exit 0; fi; app_id=\"$(/usr/libexec/PlistBuddy -c 'Print :Constants:AdMob:appIdIOS' \"$ads_plist\" 2>/dev/null || true)\"; if [ -z \"$app_id\" ] || echo \"$app_id\" | grep -q 'XXXXXXXXXXXXXXXX'; then exit 0; fi; info_plist=\"$TARGET_BUILD_DIR/$INFOPLIST_PATH\"; if [ ! -f \"$info_plist\" ]; then exit 0; fi; /usr/libexec/PlistBuddy -c \"Set :GADApplicationIdentifier $app_id\" \"$info_plist\" 2>/dev/null || /usr/libexec/PlistBuddy -c \"Add :GADApplicationIdentifier string $app_id\" \"$info_plist\""
      VERBATIM
    )
  endif()

  # ---- SKAdNetwork IDs (Chartboost/AdMob mediation) ----
  # Some SDKs log warnings if required SKAdNetwork identifiers are not present in the built Info.plist.
  # Keep the canonical list in repo and merge into the *built* Info.plist at build time.
  if(IOS)
    add_custom_command(TARGET ${APP_NAME} POST_BUILD
      COMMAND /bin/sh -c "set -e; info_plist=\"$TARGET_BUILD_DIR/$INFOPLIST_PATH\"; ids_file=\"${CMAKE_CURRENT_SOURCE_DIR}/tools/skadnetwork_chartboost_required.txt\"; script=\"${CMAKE_CURRENT_SOURCE_DIR}/tools/ensure_skadnetwork_ids.py\"; if [ ! -f \"$info_plist\" ] || [ ! -f \"$ids_file\" ] || [ ! -f \"$script\" ]; then exit 0; fi; python3 \"$script\" --info-plist \"$info_plist\" --ids-file \"$ids_file\""
      VERBATIM
    )
  endif()

  # ---- iOS AdMob (Google Mobile Ads) integration ----
  # Now handled via CocoaPods (see Podfile in build-ios-game/)
  # CocoaPods manages all linking, signing, and dependencies automatically.
  # Previous manual xcframework integration disabled to avoid conflicts with CocoaPods.

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
      COMMAND /bin/sh -c "if [ -n \"$EXPANDED_CODE_SIGN_IDENTITY\" ]; then /usr/bin/codesign --force --sign \"$EXPANDED_CODE_SIGN_IDENTITY\" --preserve-metadata=identifier,entitlements \"$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks/soft_oal.framework\"; else echo \"warning: EXPANDED_CODE_SIGN_IDENTITY is empty; soft_oal.framework left unsigned\"; fi"
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
