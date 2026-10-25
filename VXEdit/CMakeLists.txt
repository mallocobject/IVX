file(GLOB_RECURSE VXEDIT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(VXEdit ${VXEDIT_SOURCES})

# Lets sources include each other as "panel/..." and "scripts/..." regardless of
# where the including file lives.
target_include_directories(VXEdit PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(VXEdit PRIVATE IVX ImGuiColorTextEdit)

target_compile_definitions(VXEdit PRIVATE
    TEXTURES="${CMAKE_CURRENT_SOURCE_DIR}/assets/textures/"
    SHADERS="${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/"
    FONTS="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/"
    LAYOUT="${CMAKE_CURRENT_SOURCE_DIR}/assets/layout.ini"
    # script module: where the editor expects the DLL and how to rebuild it
    SCRIPTS_DLL="${CMAKE_BINARY_DIR}/VXEdit/$<CONFIG>/VXScripts.dll"
    SCRIPTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/scripts"
    IVX_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
    IVX_BUILD_DIR="${CMAKE_BINARY_DIR}"
    IVX_BUILD_CONFIG="$<CONFIG>"
    # MSBuild that CMake already selected: no runtime lookup needed
    IVX_MSBUILD="${CMAKE_MAKE_PROGRAM}")

# ---- script module (VXScripts.dll) ----
# Scripts live outside src/ on purpose: they are a separate DLL that is rebuilt
# and hot reloaded on its own, without touching the editor binary.
file(GLOB_RECURSE VXSCRIPT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.cpp")

add_library(VXScripts SHARED ${VXSCRIPT_SOURCES})

# Header-only dependency on the engine: NO target_link_libraries(VXScripts IVX),
# otherwise the static engine would be compiled into the module and every engine
# singleton (Application, logger, ...) would exist twice.
target_link_libraries(VXScripts PRIVATE IVXHeaders)

set_target_properties(VXScripts PROPERTIES
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/VXEdit/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/VXEdit/$<CONFIG>")
