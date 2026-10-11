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
    "${imgui_SOURCE_DIR}"             # <imgui.h>
    "${imgui_SOURCE_DIR}/backends"    # <backends/imgui_impl_glfw.h>
    "${CMAKE_SOURCE_DIR}/IVX/vendor") # Glad/Elog

target_link_libraries(IVX PUBLIC
    elog glfw glad imgui glm stb_image
    opengl32)

target_compile_definitions(IVX PUBLIC
    IVX_PLATFORM_WINDOWS
    GLFW_INCLUDE_NONE
    $<$<CONFIG:Debug>:IVX_CONFIG_DEBUG>
    $<$<CONFIG:Release>:IVX_CONFIG_RELEASE>)

if(IVX_INTRUSIVE_REF)
    target_compile_definitions(IVX PUBLIC IVX_INTRUSIVE_REF)
endif()