cmake_minimum_required(VERSION 3.22...4.1)

set(APP_NAME BikeRaceEscapeAxmol)

project(${APP_NAME} LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Project-specific iOS bundle id (used by Xcode generator / Info.plist).
set(PZ_BUNDLE_ID "com.nextfiction.superbikerace" CACHE STRING "Bundle identifier (iOS/macOS)")

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
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${_PZ_SOFT_OAL_SRC}" "${_PZ_SOFT_OAL_DST}"
    VERBATIM
  )

  if(TARGET OpenAL)
    add_dependencies(pz_fix_soft_oal_framework OpenAL)
  endif()
  add_dependencies(${APP_NAME} pz_fix_soft_oal_framework)

  # Ensure dyld can load it at launch
  add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${_PZ_SOFT_OAL_SRC}" "$<TARGET_FILE_DIR:${APP_NAME}>/Frameworks/soft_oal.framework"
    VERBATIM
  )
endif()

# Ship Axmol-generated runtime assets (compiled shaders) into the app bundle.
add_custom_command(TARGET ${APP_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_BINARY_DIR}/runtime
          $<TARGET_FILE_DIR:${APP_NAME}>/runtime
)

# Make sure the AXGameFinalSetup is included at the end of this file
include(AXGameFinalSetup)
