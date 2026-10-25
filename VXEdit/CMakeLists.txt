file(GLOB_RECURSE VXEDIT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(VXEdit ${VXEDIT_SOURCES})

# Lets sources include each other as "panel/..." and "scripts/..." regardless of
# where the including file lives.
target_include_directories(VXEdit PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(VXEdit PRIVATE IVX ImGuiColorTextEdit)

# The script module is NOT part of this build: it has its own standalone Ninja
# project in scripts/ that the editor configures and builds on demand, so a script
# rebuild never re-evaluates this solution (see scripts/CMakeLists.txt).
set(VX_SCRIPTS_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")
set(VX_SCRIPTS_BUILD_DIR "${CMAKE_BINARY_DIR}/scripts")

# vcvars64.bat of the Visual Studio instance CMake is using: the Ninja build of the
# script module needs the MSVC environment.
set(VX_VCVARS "${CMAKE_GENERATOR_INSTANCE}/VC/Auxiliary/Build/vcvars64.bat")
if(NOT EXISTS "${VX_VCVARS}")
    set(VX_VCVARS "")
endif()

target_compile_definitions(VXEdit PRIVATE
    TEXTURES="${CMAKE_CURRENT_SOURCE_DIR}/assets/textures/"
    SHADERS="${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/"
    FONTS="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/"
    LAYOUT="${CMAKE_CURRENT_SOURCE_DIR}/assets/layout.ini"
    # script module: where the DLL ends up and how to rebuild it
    SCRIPTS_DLL="${VX_SCRIPTS_BUILD_DIR}/VXScripts.dll"
    SCRIPTS_DIR="${VX_SCRIPTS_PROJECT_DIR}"
    SCRIPTS_PROJECT_DIR="${VX_SCRIPTS_PROJECT_DIR}"
    SCRIPTS_BUILD_DIR="${VX_SCRIPTS_BUILD_DIR}"
    IVX_ROOT_DIR="${CMAKE_SOURCE_DIR}"
    IVX_BUILD_CONFIG="$<CONFIG>"
    VCVARS_PATH="${VX_VCVARS}")
