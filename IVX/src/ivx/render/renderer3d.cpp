#include "ivxpch.h"
#include "renderer3d.h"

#include "ivx/core/logger.h"
#include "ivx/render/render_command.h"
#include "ivx/render/shadow_uniforms.h"

namespace ivx {
namespace {
struct Renderer3DData {
    Ref<Shader> default_shader;
    // Writes depth only; used by the shadow pass.
    Ref<Shader> depth_shader;
    Ref<Shader> point_depth_shader;
    bool radial_depth{false};
    // Stands in for a missing albedo map.
    Ref<Texture2D> white_texture;
    Renderer3D::Statistics stats;
};

Renderer3DData data;
} // namespace

void Renderer3D::init() {
    IVX_PROFILE_FUNCTION();

    IVX_CORE_ASSERT(
        RenderCommand::max_fragment_texture_units() >=
            k_point_shadow_texture_unit + k_max_lights,
        "Eight 2D and cube shadow slots require 18 fragment texture units");

    data.default_shader = Shader::create(SHADERS "mesh.glsl");
    data.depth_shader = Shader::create(SHADERS "shadow.glsl");
    data.point_depth_shader = Shader::create(SHADERS "point_shadow.glsl");

    data.white_texture = Texture2D::create(1, 1);
    const uint32_t white = 0xffffffffu;
    data.white_texture->set_data(&white, sizeof(white));
}

void Renderer3D::shutdown() {
    IVX_PROFILE_FUNCTION();

    data.white_texture.reset();
    data.depth_shader.reset();
    data.point_depth_shader.reset();
    data.radial_depth = false;
    data.default_shader.reset();
}

void Renderer3D::begin() {
    IVX_PROFILE_FUNCTION();

    reset_stats();
}

void Renderer3D::end() {
    IVX_PROFILE_FUNCTION();
}

void Renderer3D::submit(const Ref<Mesh> &mesh,
                        const Material &material,
                        const glm::mat4 &transform,
                        uint32_t uid) {
    IVX_PROFILE_FUNCTION();

    if (!mesh) {
        return;
    }

    const Ref<Shader> &shader =
        material.shader ? material.shader : data.default_shader;
    shader->bind();
    shader->set("u_Object.model", transform);
    shader->set("u_Object.normal_matrix",
                glm::transpose(glm::inverse(glm::mat3{transform})));
    shader->set("u_Object.albedo", material.albedo);
    shader->set("u_Object.tiling", material.tiling_factor);
    shader->set("u_Object.uid", uid);

    const Ref<Texture2D> &albedo_map =
        material.albedo_map ? material.albedo_map : data.white_texture;
    albedo_map->bind(0);

    mesh->get_vertex_array()->bind();
    RenderCommand::draw(mesh->get_vertex_array(), mesh->get_index_count());

    data.stats.draw_calls++;
    data.stats.triangles += mesh->get_index_count() / 3;
}

void Renderer3D::reset_stats() {
    data.stats = Statistics{};
}

const Renderer3D::Statistics &Renderer3D::get_stats() {
    return data.stats;
}

void Renderer3D::submit_depth(const Ref<Mesh> &mesh,
                              const glm::mat4 &transform) {
    IVX_PROFILE_FUNCTION();

    if (!mesh) {
        return;
    }

    const auto &shader =
        data.radial_depth ? data.point_depth_shader : data.depth_shader;
    shader->bind();
    shader->set("u_Model", transform);

    mesh->get_vertex_array()->bind();
    RenderCommand::draw(mesh->get_vertex_array(), mesh->get_index_count());

    data.stats.draw_calls++;
    data.stats.triangles += mesh->get_index_count() / 3;
}

void Renderer3D::begin_depth(const DepthDrawContext &context) {
    data.radial_depth =
        context.encoding == DepthDrawContext::Encoding::RadialReverseZ;
    if (data.radial_depth) {
        data.point_depth_shader->bind();
        data.point_depth_shader->set("u_LightPosition", context.light_position);
        data.point_depth_shader->set("u_Near", context.near_plane);
        data.point_depth_shader->set("u_Far", context.far_plane);
    }
}

void Renderer3D::end_depth() {
    data.radial_depth = false;
}
} // namespace ivx
