#include "ivxpch.h"
#include "renderer.h"
#include "render_command.h"

namespace ivx {
void Renderer::begin_scene() {
}

void Renderer::end_scene() {
}

void Renderer::submit(const std::shared_ptr<VertexArray> &vertex_array) {
    vertex_array->bind();
    RenderCommand::draw(vertex_array);
}
} // namespace ivx