cmake_minimum_required(VERSION 3.20)
project(BikeRaceEscapeAxmol LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    thirdparty/legacy-box2d/*.cpp
)

add_executable(BikeRaceEscapeAxmol ${GAME_SOURCES} ${LEGACY_BOX2D_SOURCES})

target_include_directories(BikeRaceEscapeAxmol PRIVATE
    Source
    thirdparty/legacy-box2d
)

target_link_libraries(BikeRaceEscapeAxmol PRIVATE axmol)

set_target_properties(BikeRaceEscapeAxmol PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(TARGET BikeRaceEscapeAxmol POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Content
            $<TARGET_FILE_DIR:BikeRaceEscapeAxmol>/Content
)
