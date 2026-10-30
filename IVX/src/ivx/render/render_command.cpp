#include "ivxpch.h"
#include "render_command.h"
#include "ivx/core/ref.hpp"
#include "renderer_api.h"

namespace ivx {
Ref<RendererAPI> RenderCommand::renderer_api_ = RendererAPI::create();
RenderCommand::State RenderCommand::state_{};

void RenderCommand::init() {
    renderer_api_->init();
    // RendererAPI::init() enables all four; keep the tracked copy in sync so
    // the first RenderPass restores a truthful state.
    state_ = State{};
}

void RenderCommand::set_viewport(int x, int y, int width, int height) {
    renderer_api_->set_viewport(x, y, width, height);
}

void RenderCommand::set_clear_color(const glm::vec4 &color) {
    renderer_api_->set_clear_color(color);
}

void RenderCommand::set_depth_test(bool enabled) {
    state_.depth_test = enabled;
    renderer_api_->set_depth_test(enabled);
}

void RenderCommand::set_depth_write(bool enabled) {
    state_.depth_write = enabled;
    renderer_api_->set_depth_write(enabled);
}

void RenderCommand::set_blend(bool enabled) {
    state_.blend = enabled;
    renderer_api_->set_blend(enabled);
}

void RenderCommand::set_cull_faces(bool enabled) {
    state_.cull_faces = enabled;
    renderer_api_->set_cull_faces(enabled);
}

void RenderCommand::clear(bool clear_color, bool clear_depth) {
    renderer_api_->clear(clear_color, clear_depth);
}

void RenderCommand::draw(const Ref<VertexArray> &vertex_array) {
    renderer_api_->draw(vertex_array);
}

void RenderCommand::draw(const Ref<VertexArray> &vertex_array,
                         uint32_t num_indices) {
    renderer_api_->draw(vertex_array, num_indices);
}

} // namespace ivx
