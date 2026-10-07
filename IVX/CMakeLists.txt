file(GLOB_RECURSE IVX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(IVX STATIC ${IVX_SOURCES})

target_precompile_headers(IVX PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/ivxpch.h")

target_include_directories(IVX PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${imgui_SOURCE_DIR}"             # <imgui.h>
    "${imgui_SOURCE_DIR}/backends"    # <backends/imgui_impl_glfw.h>
    "${CMAKE_SOURCE_DIR}/IVX/vendor") # Glad/Elog

target_link_libraries(IVX PUBLIC
    elog glfw glad imgui glm
    opengl32)

target_compile_definitions(IVX PUBLIC
    IVX_PLATFORM_WINDOWS
    GLFW_INCLUDE_NONE
    $<$<CONFIG:Debug>:IVX_CONFIG_DEBUG>
    $<$<CONFIG:Release>:IVX_CONFIG_RELEASE>)