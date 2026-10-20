#include "ivxpch.h"
#include "opengl_context.h"

#include <glad/glad.h>

// #include <gl/GL.h>

namespace ivx {
OpenGLContext::OpenGLContext(GLFWwindow *window_handle)
    : window_handle_(window_handle) {
    IVX_CORE_ASSERT(window_handle, "Window handle is null!")
}

void OpenGLContext::init() {
    IVX_PROFILE_FUNCTION();

    glfwMakeContextCurrent(window_handle_);
    int status =
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    IVX_CORE_ASSERT(status, "Failed to initialize Glad!");

    IVX_CORE_INFO("{:-^40}", "OpenGL Info");
    IVX_CORE_INFO("    Vendor: {}",
                  reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
    IVX_CORE_INFO("    Renderer: {}",
                  reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    IVX_CORE_INFO("    Version: {}",
                  reinterpret_cast<const char *>(glGetString(GL_VERSION)));

    if (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 5)) {
        IVX_CORE_FATAL("OpenGL 4.5+ is required, but the context is {}.{}. "
                       "Update your graphics driver or lower the requirement.",
                       GLVersion.major,
                       GLVersion.minor);
        IVX_CORE_ASSERT(false, "Unsupported OpenGL version!");
    }
}

void OpenGLContext::swap_buffer() {
    IVX_PROFILE_FUNCTION();

    glfwSwapBuffers(window_handle_);
}

// void OpenGLContext::clear() {
//     glClearColor(0, 0, 0, 0);
//     glClear(GL_COLOR_BUFFER_BIT);
// }
} // namespace ivx
