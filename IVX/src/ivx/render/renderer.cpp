#include "ivxpch.h"
#include "renderer.h"
#include "render_command.h"
#include <memory>

namespace ivx {
void Renderer::begin_scene() {
}

void Renderer::end_scene() {
}

void Renderer::submit(const std::shared_ptr<Shader> &shader,
                      const std::shared_ptr<VertexArray> &vertex_array) {
    shader->bind();
    vertex_array->bind();
    RenderCommand::draw(vertex_array);
    shader->unbind();
    vertex_array->unbind();
}
} // namespace ivx