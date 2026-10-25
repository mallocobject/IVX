# Standalone build for the script module (see ScriptHost in the editor).
#
# Kept out of the main build on purpose: it is configured with Ninja into its own
# build directory, so rebuilding a script never re-evaluates the editor's solution
# and never touches MSBuild's incremental state. A no-op rebuild is ~0.1 s and a
# single edited script is compile + link only.
#
# The editor configures and builds this project on demand:
#   cmake -S VXEdit/scripts -B build/scripts -G Ninja -DIVX_ROOT=<repo> ...
#   cmake --build build/scripts
#
# Requires the MSVC environment (vcvars64.bat); the editor provides it.
cmake_minimum_required(VERSION 3.21)

project(VXScripts LANGUAGES CXX)

if(NOT DEFINED IVX_ROOT)
    message(FATAL_ERROR "IVX_ROOT must point at the repository root")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# The sources are collected with a glob, and CONFIGURE_DEPENDS makes Ninja
# re-run CMake by itself when a script is added or removed.
file(GLOB SCRIPT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(VXScripts SHARED ${SCRIPT_SOURCES})
set_target_properties(VXScripts PROPERTIES PREFIX "")

# Mirrors the IVXHeaders interface target of the main build: everything a script
# needs from the engine is header-only, so only include dirs and the ABI-relevant
# defines are required. The module must not link the engine library (that would
# duplicate engine state inside the DLL).
target_include_directories(VXScripts PRIVATE
    "${IVX_ROOT}/IVX/src"
    "${IVX_ROOT}/deps/glm"
    "${IVX_ROOT}/IVX/vendor/entt/include")

target_compile_definitions(VXScripts PRIVATE
    IVX_PLATFORM_WINDOWS
    $<$<CONFIG:Debug>:IVX_CONFIG_DEBUG>
    $<$<CONFIG:Release>:IVX_CONFIG_RELEASE>
    IVX_INTRUSIVE_REF)

if(MSVC)
    target_compile_options(VXScripts PRIVATE /W4 /permissive- /Zc:preprocessor)
endif()
