file(GLOB_RECURSE IVX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Implementation behind ivx::Ref:
#   ON  -> intrusive reference counting (the count lives in RefCounted)
#   OFF -> std::shared_ptr
# The switch is a preprocessor presence macro and is PUBLIC on purpose: Ref is
# 8 bytes wide in intrusive mode and 16 in shared_ptr mode, so the library and
# every consumer (Sandbox included) must compile with the same setting.
#
# Flipping this option requires a clean rebuild: MSBuild does not recompile
# objects when only the compile definitions change, and linking stale objects
# from the other mode fails with LNK1163 / ODR violations.
option(IVX_INTRUSIVE_REF "Use the intrusive ivx::Ref implementation" ON)

add_library(IVX STATIC ${IVX_SOURCES})

target_precompile_headers(IVX PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/ivxpch.h")

target_include_directories(IVX PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/deps/imgui"             # <imgui.h>
    "${CMAKE_SOURCE_DIR}/deps/imgui/backends"    # <backends/imgui_impl_glfw.h>
    "${CMAKE_SOURCE_DIR}/deps/BqLog/include"     # <bq_log/bq_log.h>
    "${CMAKE_SOURCE_DIR}/deps/SDL/include")      # <SDL3/SDL.h>

target_link_libraries(IVX PUBLIC
    glfw glad imgui glm stb_image BqLog opengl32)

target_compile_definitions(IVX PUBLIC
    IVX_PLATFORM_WINDOWS
    GLFW_INCLUDE_NONE
    $<$<CONFIG:Debug>:IVX_CONFIG_DEBUG>
    $<$<CONFIG:Release>:IVX_CONFIG_RELEASE>)

target_compile_definitions(IVX PRIVATE
    TEXTURES="${CMAKE_CURRENT_SOURCE_DIR}/../Sandbox/assets/textures/"
    SHADERS="${CMAKE_CURRENT_SOURCE_DIR}/../Sandbox/assets/shaders/")

if(IVX_INTRUSIVE_REF)
    target_compile_definitions(IVX PUBLIC IVX_INTRUSIVE_REF)
endif()

# ---- Window/input back end ----
# OFF (default): GLFW, the current implementation.
# ON: SDL3 (SDL_Window + SDL_GLContext + event queue). SDL3 is always compiled
# as a dependency; this switch only decides which platform layer IVX uses.
option(IVX_WINDOW_SDL3 "Use SDL3 instead of GLFW for windowing/input" OFF)

if(IVX_WINDOW_SDL3)
    target_compile_definitions(IVX PUBLIC IVX_WINDOW_SDL3)
    target_link_libraries(IVX PUBLIC SDL3::SDL3)
endif()