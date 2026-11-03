#include "ivxpch.h"
#include "shadow_pass.h"

#include "ivx/utils/math.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace ivx {
namespace {
static constexpr uint32_t k_min_resolution = 256;
static constexpr float k_shadow_distance = 60.f;
static constexpr float k_normal_bias_texels = 0.5f;

int find_shadow_light(const LightSet &lights) {
    for (uint32_t i = 0; i < std::min(lights.count, k_max_lights); ++i) {
        const auto &settings = lights.lights[i].settings;
        if (settings.cast_shadows &&
            settings.type == LightType::IVX_Directional) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool camera_slice(const Camera &view, std::array<glm::vec3, 8> &corners) {
    const float near_plane = view.get_near();
    const float far_plane = view.get_far();
    const float shadow_far = std::min(far_plane, k_shadow_distance);
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
    FrameBufferSpecification target;
    target.width = target.height = resolution_;
    target.num_samples =
        1; // This implementation samples an ordinary sampler2D.
    target.depth_format = FrameBufferDepthFormat::Depth32F;

    RenderScopeSpecification spec;
    spec.target = target;
    spec.clear_color_attachments = false;
    spec.clear_depth = true;
    spec.blend.enabled = false;
    spec.cull = RendererAPI::CullMode::IVX_Back;
    spec.depth_test = spec.depth_write = true;
    spec.depth_func = RendererAPI::Cmp::IVX_Greater;
    scope_ = make_ref<RenderScope>(std::move(spec));
}

bool ShadowPass::render(Scene &scene,
                        const LightSet &lights,
                        const Camera &view) {
    gpu_ = GpuShadow{}; // No light or invalid fit must disable stale shadows.
    const int caster = find_shadow_light(lights);
    if (caster < 0 || !fit_directional(lights.lights[caster], view)) {
        return false;
    }
    gpu_.light_index.x = caster;

    scope_->begin(&light_camera_);
    scene.render(Scene::RenderMode::IVX_DepthOnly);
    scope_->end();
    return true;
}

bool ShadowPass::fit_directional(const Light &light, const Camera &view) {
    std::array<glm::vec3, 8> corners;
    const float direction_length = glm::length(light.direction);
    if (!std::isfinite(direction_length) || direction_length < 1e-6f ||
        !camera_slice(view, corners)) {
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
    gpu_.light_space = light_projection * light_view;

    const float depth_per_texel = world_texel / (far_plane - near_plane);
    // World-space offsets and projected-depth bias use different units.
    gpu_.params = {
        1.f, depth_per_texel * 0.25f, world_texel, k_normal_bias_texels};
    gpu_.params2 = {depth_per_texel * 2.5f, 0.f, 0.f, 0.f};
    return true;
}
} // namespace ivx
