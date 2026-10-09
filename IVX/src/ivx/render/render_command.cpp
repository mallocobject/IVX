#include "ivxpch.h"
#include "render_command.h"

#include "backend/opengl/opengl_renderer_api.h"

namespace ivx {
RendererAPI *RenderCommand::renderer_api_ = new OpenGLRendererAPI;

void RenderCommand::set_clear_color(const glm::vec4 &color) {
    renderer_api_->set_clear_color(color);
}

void RenderCommand::clear() {
    renderer_api_->clear();
}

void RenderCommand::draw(const std::shared_ptr<VertexArray> &vertex_array) {
    renderer_api_->draw(vertex_array);
}
} // namespace ivx