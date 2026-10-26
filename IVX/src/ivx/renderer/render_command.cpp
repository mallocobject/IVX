#include "ivxpch.h"
#include "render_command.h"
#include "ivx/core/ref.hpp"
#include "renderer_api.h"

namespace ivx {
Ref<RendererAPI> RenderCommand::renderer_api_ = RendererAPI::create();

void RenderCommand::init() {
    renderer_api_->init();
}

void RenderCommand::set_viewport(int x, int y, int width, int height) {
    renderer_api_->set_viewport(x, y, width, height);
}

void RenderCommand::set_clear_color(const glm::vec4 &color) {
    renderer_api_->set_clear_color(color);
}
void RenderCommand::set_depth_test(bool enabled) {
    renderer_api_->set_depth_test(enabled);
}

void RenderCommand::clear() {
    renderer_api_->clear();
}

void RenderCommand::draw(const Ref<VertexArray> &vertex_array) {
    renderer_api_->draw(vertex_array);
}

void RenderCommand::draw(const Ref<VertexArray> &vertex_array,
                         uint32_t num_indices) {
    renderer_api_->draw(vertex_array, num_indices);
}

} // namespace ivx
