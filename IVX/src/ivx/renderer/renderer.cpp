#include "ivxpch.h"
#include "renderer.h"
#include "ivx/renderer/camera.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/renderer/render_command.h"
#include <memory>

namespace ivx {
Renderer::SceneData *Renderer::scene_data_ = new Renderer::SceneData{
    .view_proj_matrix = glm::mat4{1.f},
};

void Renderer::init() {
    RenderCommand::init();
}

void Renderer::on_window_resize(WindowResizeEvent &event) {
    RenderCommand::set_viewport(0, 0, event.get_width(), event.get_height());
}

void Renderer::begin_scene(const Ref<OrthoGraphicCamera> &camera) {
    scene_data_->view_proj_matrix =
        camera->get_proj_matrix() * camera->get_view_matrix();
}

void Renderer::end_scene() {
}

void Renderer::submit(const Ref<Shader> &shader,
                      const Ref<VertexArray> &vertex_array,
                      const glm::mat4 &transform) {
    auto mvp = scene_data_->view_proj_matrix * transform;
    shader->set("mvp", mvp);
    shader->bind();
    vertex_array->bind();
    RenderCommand::draw(vertex_array);
    shader->unbind();
    vertex_array->unbind();
}

} // namespace ivx
