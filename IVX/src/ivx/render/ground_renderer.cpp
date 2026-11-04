#include "ivxpch.h"
#include "ground_renderer.h"

#include "ivx/render/camera.h"
#include "ivx/render/render_command.h"
#include <cmath>

namespace ivx {
void GroundRenderer::render(const Camera &camera,
                            const GroundSettings &settings) {
    if (!settings.enabled || !std::isfinite(settings.height) ||
        !std::isfinite(settings.grid_spacing) || settings.grid_spacing <= 0.f) {
        return;
    }

    if (!shader_) {
        shader_ = Shader::create(SHADERS "ground.glsl");
        triangle_ = VertexArray::create();
        uint32_t indices[]{0, 1, 2};
        triangle_->set_index_buffer(IndexBuffer::create(indices, 3));
    }

    const auto saved = RenderCommand::state();
    auto state = saved;
    state.depth_test = state.depth_write = true;
    state.depth_func = RendererAPI::Cmp::IVX_Greater;
    state.blend.enabled = false;
    state.cull = RendererAPI::CullMode::IVX_None;
    RenderCommand::set_state(state);

    shader_->bind();
    shader_->set("u_InverseVP", glm::inverse(camera.get_view_projection()));
    shader_->set("u_ViewProjection", camera.get_view_projection());
    shader_->set("u_Height", settings.height);
    shader_->set("u_GridSpacing", settings.grid_spacing);
    shader_->set("u_Color", settings.color);
    shader_->set("u_GridColor", settings.grid_color);
    triangle_->bind();
    RenderCommand::draw(triangle_);

    RenderCommand::set_state(saved);
}
} // namespace ivx
