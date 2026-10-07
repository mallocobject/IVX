# 引擎:静态库
file(GLOB_RECURSE IVX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(IVX STATIC ${IVX_SOURCES})

target_precompile_headers(IVX PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/ivxpch.h")

target_include_directories(IVX PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${VENDOR}")

# PUBLIC:静态库不做链接,依赖要传给最终链接的 Sandbox
target_link_libraries(IVX PUBLIC
    elog glfw glad imgui glm
    opengl32)

target_compile_definitions(IVX PUBLIC
    IVX_PLATFORM_WINDOWS
    GLFW_INCLUDE_NONE
    $<$<CONFIG:Debug>:IVX_CONFIG_DEBUG>
    $<$<CONFIG:Release>:IVX_CONFIG_RELEASE>)
