file(GLOB_RECURSE SANDBOX_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(IVXEditor ${SANDBOX_SOURCES})

target_link_libraries(IVXEditor PRIVATE IVX)

target_compile_definitions(IVXEditor PRIVATE
    TEXTURES="${CMAKE_CURRENT_SOURCE_DIR}/assets/textures/"
    SHADERS="${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/")