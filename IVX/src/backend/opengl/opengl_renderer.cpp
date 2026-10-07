#include "ivxpch.h"
#include "backend/opengl/opengl_renderer.h"

// #include <gl/gl.h>
#include <glad/glad.h>

namespace ivx {
void OpenGLRenderer::set_clear_color(const glm::vec4 &color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRenderer::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::draw() {
    // glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid
    // *indices)
}
} // namespace ivx