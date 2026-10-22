# cmake -S . -B build -G "Visual Studio 18 2026" -A x64

cmake_minimum_required(VERSION 3.21)
project(IVX LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/utf-8 /W3 /Zc:__cplusplus)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()

set(DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")

if (NOT EXISTS "${DEPS_DIR}/glfw" OR
    NOT EXISTS "${DEPS_DIR}/glm"  OR
    NOT EXISTS "${DEPS_DIR}/imgui")
    message(FATAL_ERROR
        "Missing dependencies in ${DEPS_DIR}.\n"
        "Please run setup_deps.bat(Windows) or ./setup_deps.sh(Linux/macOS)")
endif()

# ---- GLFW ----
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
add_subdirectory("${DEPS_DIR}/glfw" "${CMAKE_BINARY_DIR}/_deps/glfw-build")

# ---- GLM ----
set(GLM_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL  OFF CACHE BOOL "" FORCE)
add_subdirectory("${DEPS_DIR}/glm" "${CMAKE_BINARY_DIR}/_deps/glm-build")

# ---- ImGui ----
set(IMGUI_DIR "${DEPS_DIR}/imgui")
add_library(imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp")
target_include_directories(imgui PUBLIC
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends")

set(VENDOR_DIR "${CMAKE_SOURCE_DIR}/IVX/vendor")

# ---- BqLog ----
set(BUILD_LIB_TYPE static_lib CACHE STRING "" FORCE)
set(BQ_WERROR OFF CACHE BOOL "" FORCE)
set(BQ_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
add_subdirectory("${DEPS_DIR}/BqLog" "${CMAKE_BINARY_DIR}/_deps/bqlog-build")

# ---- Glad ----
add_library(glad STATIC "${VENDOR_DIR}/Glad/src/glad.c")
target_include_directories(glad PUBLIC "${VENDOR_DIR}/Glad/include")

# ---- stb_image ----
add_library(stb_image STATIC "${VENDOR_DIR}/stb_image/stb_image.cpp")
target_include_directories(stb_image PUBLIC "${VENDOR_DIR}/stb_image")

add_subdirectory(IVX)
add_subdirectory(VXEdit)
# add_subdirectory(Sandbox)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT VXEdit)
