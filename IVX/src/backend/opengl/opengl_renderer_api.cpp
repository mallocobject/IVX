#include "ivxpch.h"
#include "opengl_renderer_api.h"

// #include <gl/gl.h>
#include "ivx/core/ref.hpp"
#include <glad/glad.h>

namespace ivx {
void OpenGLRendererAPI::init() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void OpenGLRendererAPI::set_clear_color(const glm::vec4 &color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRendererAPI::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererAPI::draw(const Ref<VertexArray> &vertex_array) {
    glDrawElements(GL_TRIANGLES,
                   vertex_array->get_index_buffer()->get_count(),
                   GL_UNSIGNED_INT,
                   nullptr);
}

} // namespace ivx