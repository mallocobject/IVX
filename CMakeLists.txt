# cmake -S . -B build -G "Visual Studio 18 2026" -A x64

cmake_minimum_required(VERSION 3.21)
project(IVX LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/utf-8 /W3 /Zc:__cplusplus)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX)

    # Dynamic CRT (/MD + /MDd) for every target, third-party ones included. This has to be
    # set before the dependencies are added: yaml-cpp defaults to the *static* CRT when it
    # builds a static library, which then fails to link against our /MD objects with
    # LNK2038 "RuntimeLibrary mismatch: MTd_StaticDebug vs MDd_DynamicDebug".
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")

if (NOT EXISTS "${DEPS_DIR}/glm"  OR
    NOT EXISTS "${DEPS_DIR}/imgui" OR
    NOT EXISTS "${DEPS_DIR}/SDL" OR
    NOT EXISTS "${DEPS_DIR}/yaml-cpp")
    message(FATAL_ERROR
        "Missing dependencies in ${DEPS_DIR}.\n"
        "Please run setup_deps.bat(Windows) or ./setup_deps.sh(Linux/macOS)")
endif()

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

# ---- SDL3 ----
# Only the static library is needed; tests, examples and the test library are
# off so the solution stays small. SDL3::SDL3 resolves to SDL3-static here.
set(SDL_SHARED       OFF CACHE BOOL "" FORCE)
set(SDL_STATIC       ON  CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS        OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES     OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL      OFF CACHE BOOL "" FORCE)
add_subdirectory("${DEPS_DIR}/SDL" "${CMAKE_BINARY_DIR}/_deps/sdl3-build")

# ---- Glad ----
add_library(glad STATIC "${VENDOR_DIR}/Glad/src/glad.c")
target_include_directories(glad PUBLIC "${VENDOR_DIR}/Glad/include")

# ---- stb_image ----
add_library(stb_image STATIC "${VENDOR_DIR}/stb_image/stb_image.cpp")
target_include_directories(stb_image PUBLIC "${VENDOR_DIR}/stb_image")

# ---- entt ----
add_library(entt INTERFACE)
target_include_directories(entt INTERFACE "${VENDOR_DIR}/entt/include")

# ---- yaml-cpp ----
# Only the library is needed: the parse/read/sandbox command line tools and the googletest
# suite would be built (and take minutes) for nothing otherwise.
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${DEPS_DIR}/yaml-cpp" "${CMAKE_BINARY_DIR}/_deps/yaml-cpp-build")

# ---- ImGuiColorTextEdit ----
# Syntax highlighting editor widget used by the editor's script panel.
# BalazsJako's original API: TextEditor.h / TextEditor.cpp (MIT).
add_library(ImGuiColorTextEdit STATIC
    "${VENDOR_DIR}/ImGuiColorTextEdit/TextEditor.cpp")
target_include_directories(ImGuiColorTextEdit PUBLIC
    "${VENDOR_DIR}/ImGuiColorTextEdit")
target_link_libraries(ImGuiColorTextEdit PUBLIC imgui)

add_subdirectory(IVX)
add_subdirectory(VXEdit)
# add_subdirectory(Sandbox)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT VXEdit)
