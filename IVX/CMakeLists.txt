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
    "${CMAKE_SOURCE_DIR}/deps/imgui/backends"    # <backends/imgui_impl_sdl3.h>
    "${CMAKE_SOURCE_DIR}/deps/BqLog/include"     # <bq_log/bq_log.h>
    "${CMAKE_SOURCE_DIR}/deps/SDL/include"       # <SDL3/SDL.h>
    "${CMAKE_SOURCE_DIR}/deps/yaml-cpp/include") # <yaml-cpp/yaml.h>

# The namespaced alias, not the bare target name: a typo in a plain name is silently
# handed to the linker as a file name ("LNK1104: cannot open file YAML_CPP.lib"), while
# a wrong namespaced name fails at configure time.
target_link_libraries(IVX PUBLIC
    SDL3::SDL3 glad imgui glm stb_image BqLog opengl32 entt yaml-cpp::yaml-cpp comdlg32)

target_compile_definitions(IVX PUBLIC
    IVX_PLATFORM_WINDOWS
    $<$<CONFIG:Debug>:IVX_CONFIG_DEBUG>
    $<$<CONFIG:Release>:IVX_CONFIG_RELEASE>)

target_compile_definitions(IVX PRIVATE
    TEXTURES="${CMAKE_CURRENT_SOURCE_DIR}/../Sandbox/assets/textures/"
    SHADERS="${CMAKE_CURRENT_SOURCE_DIR}/../Sandbox/assets/shaders/")

if(IVX_INTRUSIVE_REF)
    target_compile_definitions(IVX PUBLIC IVX_INTRUSIVE_REF)
endif()

# SDL3 is the only window/input back end: SDL_Window + SDL_GLContext + event
# queue. It is linked above, together with the rest of the engine's dependencies.
# ---- script modules ----
# Everything a script module needs from the engine is header-only (Entity,
# components, ScriptableEntity, EnTT). This interface target carries only include
# dirs and the ABI-relevant compile definitions, so a module never links the
# static engine library (that would duplicate engine state inside the DLL).
add_library(IVXHeaders INTERFACE)
target_include_directories(IVXHeaders INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/src")
# glm and ennt are header-only for our use: depend on the include dirs, not on the
# library targets, so building a script module does not pull any other project
# into the build (that is what made an incremental script rebuild take seconds).
target_include_directories(IVXHeaders INTERFACE
    "${CMAKE_SOURCE_DIR}/deps/glm"
    "${VENDOR_DIR}/entt/include")
target_compile_definitions(IVXHeaders INTERFACE
    IVX_PLATFORM_WINDOWS
    $<$<CONFIG:Debug>:IVX_CONFIG_DEBUG>
    $<$<CONFIG:Release>:IVX_CONFIG_RELEASE>)
if(IVX_INTRUSIVE_REF)
    target_compile_definitions(IVXHeaders INTERFACE IVX_INTRUSIVE_REF)
endif()
