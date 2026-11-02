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

GLenum to_gl(RendererAPI::Cmp func) {
    using enum RendererAPI::Cmp;
    switch (func) {
    case IVX_Never:
        return GL_NEVER;
    case IVX_Less:
        return GL_LESS;
    case IVX_Equal:
        return GL_EQUAL;
    case IVX_Lequal:
        return GL_LEQUAL;
    case IVX_Greater:
        return GL_GREATER;
    case IVX_Nequal:
        return GL_NOTEQUAL;
    case IVX_Gequal:
        return GL_GEQUAL;
    case IVX_Always:
    default:
        return GL_ALWAYS;
    }
}

GLenum to_gl(RendererAPI::StencilOp op) {
    using enum RendererAPI::StencilOp;
    switch (op) {
    case IVX_Zero:
        return GL_ZERO;
    case IVX_Replace:
        return GL_REPLACE;
    case IVX_IncrClamp:
        return GL_INCR;
    case IVX_DecrClamp:
        return GL_DECR;
    case IVX_Invert:
        return GL_INVERT;
    case IVX_IncrWrap:
        return GL_INCR_WRAP;
    case IVX_DecrWrap:
        return GL_DECR_WRAP;
    case IVX_Keep:
    default:
        return GL_KEEP;
    }
}

GLenum to_gl(RendererAPI::BlendFactor factor) {
    using enum RendererAPI::BlendFactor;
    switch (factor) {
    case IVX_Zero:
        return GL_ZERO;
    case IVX_One:
        return GL_ONE;
    case IVX_SrcAlpha:
        return GL_SRC_ALPHA;
    case IVX_OneMinusSrcAlpha:
        return GL_ONE_MINUS_SRC_ALPHA;
    case IVX_DstAlpha:
        return GL_DST_ALPHA;
    case IVX_OneMinusDstAlpha:
        return GL_ONE_MINUS_DST_ALPHA;
    case IVX_SrcColor:
        return GL_SRC_COLOR;
    case IVX_OneMinusSrcColor:
        return GL_ONE_MINUS_SRC_COLOR;
    case IVX_DstColor:
        return GL_DST_COLOR;
    case IVX_OneMinusDstColor:
    default:
        return GL_ONE_MINUS_DST_COLOR;
    }
}

GLenum to_gl(RendererAPI::BlendEquation equation) {
    using enum RendererAPI::BlendEquation;
    switch (equation) {
    case IVX_Subtract:
        return GL_FUNC_SUBTRACT;
    case IVX_ReverseSubtract:
        return GL_FUNC_REVERSE_SUBTRACT;
    case IVX_Min:
        return GL_MIN;
    case IVX_Max:
        return GL_MAX;
    case IVX_Add:
    default:
        return GL_FUNC_ADD;
    }
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

    // Only what no pass may change: the winding order the meshes are authored
    // with, and the clip convention every projection helper in math.h assumes.
    // Depth, stencil, blending and culling belong to the passes; the engine
    // default comes from RenderCommand::init(), which pushes State{} once.
    glFrontFace(GL_CCW);
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

void OpenGLRendererAPI::set_depth_func(Cmp func) {
    glDepthFunc(to_gl(func));
}

void OpenGLRendererAPI::set_blend(bool enabled) {
    if (enabled) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
}

void OpenGLRendererAPI::set_blend_func(BlendFactor src, BlendFactor dst) {
    glBlendFunc(to_gl(src), to_gl(dst));
}

void OpenGLRendererAPI::set_blend_equation(BlendEquation equation) {
    glBlendEquation(to_gl(equation));
}

void OpenGLRendererAPI::set_cull_mode(CullMode mode) {
    if (mode == CullMode::IVX_None) {
        glDisable(GL_CULL_FACE);
        return;
    }

    glEnable(GL_CULL_FACE);
    glCullFace(mode == CullMode::IVX_Front ? GL_FRONT : GL_BACK);
}

void OpenGLRendererAPI::set_stencil_test(bool enabled) {
    if (enabled) {
        glEnable(GL_STENCIL_TEST);
    } else {
        glDisable(GL_STENCIL_TEST);
    }
}

void OpenGLRendererAPI::set_stencil_func(Cmp func,
                                         int ref,
                                         uint32_t read_mask) {
    glStencilFunc(to_gl(func), ref, static_cast<GLuint>(read_mask));
}

void OpenGLRendererAPI::set_stencil_op(StencilOp fail,
                                       StencilOp depth_fail,
                                       StencilOp pass) {
    glStencilOp(to_gl(fail), to_gl(depth_fail), to_gl(pass));
}

void OpenGLRendererAPI::set_stencil_write_mask(uint32_t mask) {
    glStencilMask(static_cast<GLuint>(mask));
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
