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

add_subdirectory("${VENDOR}/GLFW" "${CMAKE_BINARY_DIR}/GLFW")

add_library(glad STATIC "${VENDOR}/Glad/src/glad.c")
target_include_directories(glad PUBLIC "${VENDOR}/Glad/include")

add_library(imgui STATIC
    "${VENDOR}/imgui/imgui.cpp"
    "${VENDOR}/imgui/imgui_draw.cpp"
    "${VENDOR}/imgui/imgui_tables.cpp"
    "${VENDOR}/imgui/imgui_widgets.cpp"
    "${VENDOR}/imgui/imgui_demo.cpp")

target_include_directories(imgui PUBLIC "${VENDOR}/imgui" "${VENDOR}/imgui/backends")

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "${VENDOR}/glm")

add_library(elog INTERFACE)
target_include_directories(elog INTERFACE "${VENDOR}/Elog")

add_subdirectory(IVX)
add_subdirectory(Sandbox)
