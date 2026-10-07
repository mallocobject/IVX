# cmake -S . -B build -A x64
# cmd /c '"E:\Software\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" && cmake -S . -B build-clangd --fresh -G Ninja -DCMAKE_BUILD_TYPE=Debug'

cmake_minimum_required(VERSION 3.21)
project(IVX LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/utf-8 /W3)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()

set(VENDOR "${CMAKE_CURRENT_SOURCE_DIR}/IVX/vendor")

include(FetchContent)

set(FETCHCONTENT_SOURCE_DIR_GLFW "${CMAKE_SOURCE_DIR}/deps/glfw" CACHE PATH "local glfw src")
set(FETCHCONTENT_SOURCE_DIR_GLM  "${CMAKE_SOURCE_DIR}/deps/glm"  CACHE PATH "local glm src")
set(FETCHCONTENT_SOURCE_DIR_IMGUI "${CMAKE_SOURCE_DIR}/deps/imgui" CACHE PATH "local imgui src")

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/mallocobject/glfw.git
    GIT_TAG        master)

set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/icaven/glm.git
    GIT_TAG        master)

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/mallocobject/imgui.git
    GIT_TAG        docking)

FetchContent_MakeAvailable(glfw glm)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/deps/imgui")
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