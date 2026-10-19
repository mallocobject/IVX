#include "ivxpch.h"
#include "opengl_renderer_api.h"

// #include <gl/gl.h>
#include "ivx/core/logger.h"
#include "ivx/core/ref.hpp"
#include <glad/glad.h>

namespace ivx {
namespace {
void opengl_message_callback(GLenum source,
                             GLenum type,
                             GLuint id,
                             GLenum severity,
                             GLsizei length,
                             const GLchar *message,
                             const void *userParam) {
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        IVX_CORE_FATAL("{}", message);
        return;
    case GL_DEBUG_SEVERITY_MEDIUM:
        IVX_CORE_ERROR("{}", message);
        return;
    case GL_DEBUG_SEVERITY_LOW:
        IVX_CORE_WARN("{}", message);
        return;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        IVX_CORE_TRACE("{}", message);
        return;
    }

    IVX_CORE_ASSERT(false, "Unknown severity level!");
}
} // namespace

void OpenGLRendererAPI::init() {
    IVX_PROFILE_FUNCTION();

#if defined(IVX_CONFIG_DEBUG)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(opengl_message_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE,
                          GL_DONT_CARE,
                          GL_DEBUG_SEVERITY_NOTIFICATION,
                          0,
                          NULL,
                          GL_FALSE);
#endif

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER); // reverse-z

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
}

void OpenGLRendererAPI::set_viewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void OpenGLRendererAPI::set_clear_color(const glm::vec4 &color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRendererAPI::clear() {
    glClearDepth(0.0); // reverse-z
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererAPI::draw(const Ref<VertexArray> &vertex_array) {
    glDrawElements(GL_TRIANGLES,
                   vertex_array->get_index_buffer()->get_count(),
                   GL_UNSIGNED_INT,
                   nullptr);
}

} // namespace ivx