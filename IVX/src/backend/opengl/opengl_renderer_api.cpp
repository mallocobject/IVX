#include "ivxpch.h"
#include "opengl_renderer_api.h"

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
        IVX_CORE_VERBOSE("{}", message);
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

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
}

void OpenGLRendererAPI::set_viewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void OpenGLRendererAPI::set_clear_color(const glm::vec4 &color) {
    glClearColor(color.r, color.g, color.b, color.a);
}
void OpenGLRendererAPI::set_depth_test(bool enabled) {
    if (enabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void OpenGLRendererAPI::set_depth_write(bool enabled) {
    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void OpenGLRendererAPI::set_blend(bool enabled) {
    if (enabled) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
}

void OpenGLRendererAPI::set_cull_faces(bool enabled) {
    if (enabled) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void OpenGLRendererAPI::clear(bool clear_color, bool clear_depth) {
    glClearDepth(0.0); // reverse-z
    GLbitfield mask = 0;
    if (clear_color) {
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (clear_depth) {
        mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    }
    if (mask != 0) {
        glClear(mask);
    }
}

void OpenGLRendererAPI::draw(const Ref<VertexArray> &vertex_array) {
    glDrawElements(GL_TRIANGLES,
                   vertex_array->get_index_buffer()->get_count(),
                   GL_UNSIGNED_INT,
                   nullptr);
}

void OpenGLRendererAPI::draw(const Ref<VertexArray> &vertex_array,
                             uint32_t num_indices) {
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
}

} // namespace ivx
