#include "ivxpch.h"
#include "shadow_pass.h"
#include "ivx/render/renderer3d.h"

#include "ivx/utils/math.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace ivx {
namespace {
static constexpr uint32_t k_min_resolution = 256;

bool camera_slice(const Camera &view,
                  float distance,
                  std::array<glm::vec3, 8> &corners) {
    const float near_plane = view.get_near();
    const float far_plane = view.get_far();
    const float shadow_far = std::min(far_plane, distance);
    if (!std::isfinite(near_plane) || !std::isfinite(far_plane) ||
        near_plane < 0.f || far_plane <= near_plane ||
        shadow_far <= near_plane) {
        return false;
    }
    const glm::mat4 inverse = glm::inverse(view.get_view_projection());
    const float fraction = (shadow_far - near_plane) / (far_plane - near_plane);
    for (int i = 0; i < 4; ++i) {
        const float x = (i & 1) ? 1.f : -1.f;
        const float y = (i & 2) ? 1.f : -1.f;
        glm::vec4 near_corner = inverse * glm::vec4{x, y, 1.f, 1.f};
        glm::vec4 far_corner = inverse * glm::vec4{x, y, 0.f, 1.f};
        if (std::abs(near_corner.w) < 1e-8f || std::abs(far_corner.w) < 1e-8f) {
            return false;
        }
        glm::vec3 a = glm::vec3{near_corner} / near_corner.w;
        glm::vec3 b = glm::vec3{far_corner} / far_corner.w;
        corners[i] = a;

        // a + (b - a) * fraction = (1 - fraction) * a + fraction * b
        corners[i + 4] = glm::mix(a, b, fraction);
        for (int axis = 0; axis < 3; ++axis) {
            if (!std::isfinite(corners[i][axis]) ||
                !std::isfinite(corners[i + 4][axis])) {
                return false;
            }
        }
    }
    return true;
}
} // namespace

ShadowPass::ShadowPass(uint32_t resolution)
    : resolution_(std::max(resolution, k_min_resolution)) {
    FrameBufferSpecification empty;
    empty.width = empty.height = 1;
    empty.num_samples = 1;
    empty.depth_format = FrameBufferDepthFormat::Depth32F;
    empty_map_ = FrameBuffer::create(empty);
    empty_map_->clear({}, false, true);
    empty.depth_texture_type = DepthTextureType::CubeMap;
    empty.depth_compare = true;
    empty_cube_ = FrameBuffer::create(empty);
    for (uint32_t face = 0; face < 6; ++face) {
        empty_cube_->set_depth_face(face);
        empty_cube_->clear({}, false, true);
    }
}

bool ShadowPass::render(Scene &scene,
                        const LightSet &lights,
                        const Camera &view) {
    gpu_ = GpuShadowData{};
    const auto &settings = scene.environment().shadows;
    const uint32_t resolution =
        shadow_resolution_or_default(settings.resolution);
    if (resolution_ != resolution) {
        resolution_ = resolution;
        for (auto &scope : scopes_) {
            if (scope) {
                scope->resize(resolution_, resolution_);
            }
        }
    }
    const float distance = std::isfinite(settings.distance)
                               ? std::clamp(settings.distance, 0.1f, 10000.f)
                               : 120.f;
    bool rendered = false;
    for (uint32_t i = 0; i < std::min(lights.count, k_max_lights); ++i) {
        const Light &light = lights.lights[i];
        gpu_.shadows[i].light_type = static_cast<uint32_t>(light.settings.type);
        if (!light.settings.cast_shadows) {
            continue;
        }

        bool fits = false;
        using enum LightType;
        switch (light.settings.type) {
        case IVX_Directional:
            fits = fit_directional(light, view, distance, gpu_.shadows[i]);
            break;
        case IVX_Spot:
            fits = fit_spot(light, view, distance, gpu_.shadows[i]);
            break;
        case IVX_Point:
            fits = fit_point(light, gpu_.shadows[i]);
            break;
        default:
            break;
        }
        if (!fits) {
            continue;
        }

        const bool point = light.settings.type == LightType::IVX_Point;
        const auto texture_type =
            point ? DepthTextureType::CubeMap : DepthTextureType::Texture2D;
        if (scopes_[i] &&
            scopes_[i]->target()->get_specification().depth_texture_type !=
                texture_type) {
            scopes_[i].reset();
        }
        if (!scopes_[i]) {
            FrameBufferSpecification target;
            target.width = target.height = resolution_;
            target.num_samples = 1;
            target.depth_format = FrameBufferDepthFormat::Depth32F;
            target.depth_texture_type = texture_type;
            target.depth_compare = point;
            RenderScopeSpecification spec;
            spec.target = target;
            spec.clear_color_attachments = false;
            spec.clear_depth = true;
            spec.blend.enabled = false;
            spec.cull = RendererAPI::CullMode::IVX_Back;
            spec.depth_test = spec.depth_write = true;
            spec.depth_func = RendererAPI::Cmp::IVX_Greater;
            scopes_[i] = make_ref<RenderScope>(std::move(spec));
        }
        DepthDrawContext depth;
        if (point) {
            depth.encoding = DepthDrawContext::Encoding::RadialReverseZ;
            depth.light_position = light.position;
            depth.near_plane = gpu_.shadows[i].near_plane;
            depth.far_plane = gpu_.shadows[i].far_plane;
        }
        Renderer3D::begin_depth(depth);
        for (uint32_t face = 0; face < (point ? 6u : 1u); ++face) {
            if (point) {
                scopes_[i]->target()->set_depth_face(face);
                light_camera_.set(point_views_[face],
                                  point_projection_,
                                  depth.near_plane / std::sqrt(3.f),
                                  depth.far_plane);
            }
            scopes_[i]->begin();
            camera_uniforms_.update(light_camera_);
            scene.render(Scene::RenderMode::IVX_DepthOnly);
            scopes_[i]->end();
        }
        Renderer3D::end_depth();
        rendered = true;
    }
    return rendered;
}

void ShadowPass::bind_maps() const {
    for (uint32_t i = 0; i < k_max_lights; ++i) {
        const bool enabled = gpu_.shadows[i].enabled != 0;
        const bool point = gpu_.shadows[i].light_type ==
                           static_cast<uint32_t>(LightType::IVX_Point);
        const auto &map = (enabled && !point) ? output(i) : empty_map_;
        const auto &cube = (enabled && point) ? output(i) : empty_cube_;
        map->bind_depth_attachment(k_shadow_texture_unit + i);
        cube->bind_depth_attachment(k_point_shadow_texture_unit + i);
    }
}

bool ShadowPass::fit_directional(const Light &light,
                                 const Camera &view,
                                 float distance,
                                 GpuShadow &gpu) {
    std::array<glm::vec3, 8> corners;
    const float direction_length = glm::length(light.direction);
    if (!std::isfinite(direction_length) || direction_length < 1e-6f ||
        !camera_slice(view, distance, corners)) {
        return false;
    }

    glm::vec3 center{0.f};
    for (const auto &corner : corners) {
        center += corner;
    }
    center /= 8.f;
    float radius = 0.f;
    for (const auto &corner : corners) {
        radius = std::max(radius, glm::length(corner - center));
    }
    radius = std::max(0.1f, std::ceil(radius * 10.f) / 10.f);
    radius *= 1.f + 4.f / static_cast<float>(resolution_);
    const float world_texel = 2.f * radius / static_cast<float>(resolution_);

    const glm::vec3 direction = light.direction / direction_length;
    const glm::vec3 up = std::abs(direction.y) > 0.99f
                             ? glm::vec3{0.f, 0.f, 1.f}
                             : glm::vec3{0.f, 1.f, 0.f};
    const glm::mat3 light_rotation{glm::lookAt(glm::vec3{0.f}, direction, up)};
    glm::vec3 light_center = light_rotation * center;
    light_center.x = std::round(light_center.x / world_texel) * world_texel;
    light_center.y = std::round(light_center.y / world_texel) * world_texel;
    center = glm::transpose(light_rotation) * light_center;

    const glm::mat4 light_view =
        glm::lookAt(center - direction * 2.f * radius, center, up);
    const float near_plane = radius * 0.5f;
    const float far_plane = radius * 4.f;
    const glm::mat4 light_projection = ortho_rh_reverse_z(
        -radius, radius, -radius, radius, near_plane, far_plane);
    light_camera_.set(light_view, light_projection, near_plane, far_plane);
    gpu.light_space = light_projection * light_view;

    const auto bias = [](float value, float fallback) {
        return std::isfinite(value) ? std::clamp(value, 0.f, 10.f) : fallback;
    };
    gpu.enabled = 1;
    gpu.depth_bias_texels = bias(light.settings.shadow_depth_bias, 0.25f);
    gpu.normal_bias_texels = bias(light.settings.shadow_normal_bias, 0.5f);
    gpu.slope_bias_texels = bias(light.settings.shadow_slope_bias, 2.5f);
    gpu.world_texel_size = world_texel;
    gpu.near_plane = near_plane;
    gpu.far_plane = far_plane;
    gpu.depth_scale = 1.f / (far_plane - near_plane);
    return true;
}

bool ShadowPass::fit_spot(const Light &light,
                          const Camera & /*view*/,
                          float /*distance*/,
                          GpuShadow &gpu) {
    const float direction_length = glm::length(light.direction);
    if (!std::isfinite(direction_length) || direction_length < 1e-6f ||
        !std::isfinite(light.settings.shadow_near) ||
        !std::isfinite(light.settings.shadow_range) ||
        !std::isfinite(light.settings.outer_angle)) {
        return false;
    }

    const float near_plane =
        std::clamp(light.settings.shadow_near, 0.001f, 9999.f);
    const float far_plane =
        std::clamp(light.settings.shadow_range, 0.002f, 10000.f);

    if (far_plane <= near_plane) {
        return false;
    }

    const float outer_angle =
        glm::clamp(light.settings.outer_angle, 0.1f, 89.f);

    const float fov = glm::radians(outer_angle * 2.f);

    const glm::vec3 direction = light.direction / direction_length;
    if (!std::isfinite(light.position.x) || !std::isfinite(light.position.y) ||
        !std::isfinite(light.position.z)) {
        return false;
    }
    const glm::vec3 up = std::abs(direction.y) > 0.99f
                             ? glm::vec3{0.f, 0.f, 1.f}
                             : glm::vec3{0.f, 1.f, 0.f};

    const glm::mat4 light_view =
        glm::lookAt(light.position, light.position + direction, up);

    const glm::mat4 light_projection =
        perspective_rh_reverse_z(fov, 1.f, near_plane, far_plane);

    light_camera_.set(light_view, light_projection, near_plane, far_plane);

    gpu.light_space = light_projection * light_view;

    const auto bias = [](float value, float fallback) {
        return std::isfinite(value) ? std::clamp(value, 0.f, 10.f) : fallback;
    };
    gpu.enabled = 1;
    gpu.depth_bias_texels = bias(light.settings.shadow_depth_bias, 0.25f);
    gpu.normal_bias_texels = bias(light.settings.shadow_normal_bias, 0.5f);
    gpu.slope_bias_texels = bias(light.settings.shadow_slope_bias, 2.5f);
    gpu.near_plane = near_plane;
    gpu.far_plane = far_plane;
    gpu.tan_half_fov = std::tan(fov * 0.5f);
    gpu.depth_scale = near_plane * far_plane / (far_plane - near_plane);

    return true;
}

bool ShadowPass::fit_point(const Light &light, GpuShadow &gpu) {
    if (!std::isfinite(light.settings.shadow_near) ||
        !std::isfinite(light.settings.shadow_range) ||
        !std::isfinite(light.position.x) || !std::isfinite(light.position.y) ||
        !std::isfinite(light.position.z)) {
        return false;
    }
    const float near_plane =
        std::clamp(light.settings.shadow_near, 0.001f, 9999.f);
    const float far_plane =
        std::clamp(light.settings.shadow_range, 0.002f, 10000.f);
    if (far_plane <= near_plane) {
        return false;
    }

    static const std::array<glm::vec3, 6> directions{
        {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}};
    static const std::array<glm::vec3, 6> ups{{{0, -1, 0},
                                               {0, -1, 0},
                                               {0, 0, 1},
                                               {0, 0, -1},
                                               {0, -1, 0},
                                               {0, -1, 0}}};
    // Face clipping must enclose the radial near sphere, including cube
    // corners.
    point_projection_ = perspective_rh_reverse_z(
        glm::radians(90.f), 1.f, near_plane / std::sqrt(3.f), far_plane);
    for (uint32_t face = 0; face < 6; ++face) {
        point_views_[face] = glm::lookAt(
            light.position, light.position + directions[face], ups[face]);
    }

    const auto bias = [](float value, float fallback) {
        return std::isfinite(value) ? std::clamp(value, 0.f, 10.f) : fallback;
    };
    gpu.enabled = 1;
    gpu.depth_bias_texels = bias(light.settings.shadow_depth_bias, 0.25f);
    gpu.normal_bias_texels = bias(light.settings.shadow_normal_bias, 0.5f);
    gpu.slope_bias_texels = bias(light.settings.shadow_slope_bias, 2.5f);
    gpu.near_plane = near_plane;
    gpu.far_plane = far_plane;
    gpu.tan_half_fov = 1.f;
    gpu.depth_scale = 1.f / (far_plane - near_plane);
    return true;
}

} // namespace ivx
