# cmake -S . -B build -G "Visual Studio 18 2026" -A x64

cmake_minimum_required(VERSION 3.21)
project(IVX LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/utf-8 /W3)
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

add_library(glad STATIC "${CMAKE_SOURCE_DIR}/IVX/vendor/Glad/src/glad.c")
target_include_directories(glad PUBLIC "${CMAKE_SOURCE_DIR}/IVX/vendor/Glad/include")

add_library(elog INTERFACE)
target_include_directories(elog INTERFACE "${CMAKE_SOURCE_DIR}/IVX/vendor/Elog")

add_subdirectory(IVX)
add_subdirectory(Sandbox)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Sandbox)
