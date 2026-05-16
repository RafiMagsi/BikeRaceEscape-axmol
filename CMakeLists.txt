cmake_minimum_required(VERSION 3.20)
project(BikeRaceEscapeAxmol LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# iOS bundle identifier used by Xcode generator (Info.plist uses $(PRODUCT_BUNDLE_IDENTIFIER)).
set(PZ_BUNDLE_ID "com.nextfiction.superbikerace" CACHE STRING "Bundle identifier (iOS/macOS)")

if(NOT DEFINED AX_ROOT)
    if(DEFINED ENV{AX_ROOT})
        set(AX_ROOT "$ENV{AX_ROOT}")
    else()
        message(FATAL_ERROR "AX_ROOT is required. Example: cmake -S . -B build -DAX_ROOT=$HOME/Development/axmol")
    endif()
endif()

add_subdirectory(${AX_ROOT} ${CMAKE_BINARY_DIR}/axmol)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    Source/*.cpp
    Source/*.c
    Source/*.cc
)

# Native commercial/plugin bridge is kept as reference, not compiled in the core Axmol pass.
list(FILTER GAME_SOURCES EXCLUDE REGEX ".*/StoreBridge/.*")

file(GLOB_RECURSE LEGACY_BOX2D_SOURCES CONFIGURE_DEPENDS
    thirdparty/Box2D/*.cpp
)

add_executable(BikeRaceEscapeAxmol ${GAME_SOURCES} ${LEGACY_BOX2D_SOURCES})

target_include_directories(BikeRaceEscapeAxmol PRIVATE
    Source
    thirdparty
)

target_link_libraries(BikeRaceEscapeAxmol PRIVATE axmol)

if(APPLE)
    # Ensure the generated Info.plist gets a real CFBundleIdentifier (simctl install requires it).
    set_target_properties(BikeRaceEscapeAxmol PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "${PZ_BUNDLE_ID}"
    )
endif()

if(IOS)
    set_target_properties(BikeRaceEscapeAxmol PROPERTIES
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${PZ_BUNDLE_ID}"
    )
endif()

# Axmol's iOS OpenAL build produces `soft_oal.framework` under `${CMAKE_BINARY_DIR}/build/<cfg>-<sdk>/`,
# but the link line for some configs expects it under `${CMAKE_BINARY_DIR}/axmol/engine/3rdparty/openal/<cfg>-<sdk>/`.
# Ensure the expected path exists by copying the framework before linking the app.
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

    # Ensure OpenAL is built before attempting to copy its framework output.
    if(TARGET OpenAL)
        add_dependencies(pz_fix_soft_oal_framework OpenAL)
    endif()
    add_dependencies(BikeRaceEscapeAxmol pz_fix_soft_oal_framework)
endif()

set_target_properties(BikeRaceEscapeAxmol PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(TARGET BikeRaceEscapeAxmol POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Content
            $<TARGET_FILE_DIR:BikeRaceEscapeAxmol>/Content
)
