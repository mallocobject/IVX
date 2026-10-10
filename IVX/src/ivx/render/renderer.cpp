#include "ivxpch.h"
#include "renderer.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "ortho_graphic_camera.h"
#include "render_command.h"
#include <memory>

namespace ivx {
void Renderer::begin_scene(OrthoGraphicCamera &camera) {
    vp_ = camera.get_proj_matrix() * camera.get_view_matrix();
}

void Renderer::end_scene() {
}

void Renderer::submit(const std::shared_ptr<Shader> &shader,
                      const std::shared_ptr<VertexArray> &vertex_array,
                      const glm::mat4 &transform) {
    auto mvp = vp_ * transform;
    shader->set("mvp", mvp);
    shader->bind();
    vertex_array->bind();
    RenderCommand::draw(vertex_array);
    shader->unbind();
    vertex_array->unbind();
}
} // namespace ivx