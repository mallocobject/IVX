#include "ivxpch.h"
#include "renderer3d.h"

#include "ivx/core/logger.h"
#include "ivx/render/render_command.h"

namespace ivx {
namespace {
struct Renderer3DData {
    Ref<Shader> default_shader;
    glm::mat4 view_projection{1.f};
    glm::vec3 light_direction{-0.4f, -1.f, -0.6f};
    Renderer3D::Statistics stats;
};

Renderer3DData data;
} // namespace

void Renderer3D::init() {
    IVX_PROFILE_FUNCTION();

    data.default_shader = Shader::create(SHADERS "mesh.glsl");
}

void Renderer3D::shutdown() {
    IVX_PROFILE_FUNCTION();

    data.default_shader.reset();
}

void Renderer3D::begin_batch(const Camera &camera) {
    IVX_PROFILE_FUNCTION();

    data.view_projection = camera.get_proj_matrix() * camera.get_view_matrix();
    data.default_shader->set("u_VP", data.view_projection);
    data.default_shader->set("u_LightDirection", data.light_direction);
    data.default_shader->set("u_Uid", 0u);
    reset_stats();
}

void Renderer3D::end_batch() {
    IVX_PROFILE_FUNCTION();
}

void Renderer3D::submit(const Ref<Mesh> &mesh,
                        const Ref<Shader> &shader,
                        const glm::mat4 &transform,
                        const glm::vec4 &tint,
                        uint32_t uid) {
    IVX_PROFILE_FUNCTION();

    if (!mesh || !shader) {
        return;
    }

    shader->bind();
    shader->set("u_VP", data.view_projection);
    shader->set("u_Model", transform);
    // Inverse transpose: right for non uniform scale, unlike the model matrix
    // itself.
    shader->set("u_NormalMatrix",
                glm::transpose(glm::inverse(glm::mat3{transform})));
    shader->set("u_Color", tint);
    shader->set("u_Uid", uid);

    mesh->get_vertex_array()->bind();
    RenderCommand::draw(mesh->get_vertex_array(), mesh->get_index_count());

    data.stats.draw_calls++;
    data.stats.triangles += mesh->get_index_count() / 3;
}

void Renderer3D::submit(const Ref<Mesh> &mesh,
                        const glm::mat4 &transform,
                        const glm::vec4 &color,
                        uint32_t uid) {
    submit(mesh, data.default_shader, transform, color, uid);
}

void Renderer3D::reset_stats() {
    data.stats = Statistics{};
}

const Renderer3D::Statistics &Renderer3D::get_stats() {
    return data.stats;
}

} // namespace ivx
