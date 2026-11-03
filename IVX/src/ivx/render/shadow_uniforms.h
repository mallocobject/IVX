#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/light.h"
#include "ivx/render/uniform_buffer.h"
#include <array>

#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {
struct alignas(16) GpuShadow {
    glm::mat4 light_space{1.f}; // world -> light clip
    uint32_t enabled{0};
    uint32_t light_type{0}; // LightType
    float depth_bias_texels{0.f};
    float normal_bias_texels{0.f};

    float slope_bias_texels{0.f};
    float world_texel_size{0.f}; // Constant for directional lights.
    float near_plane{0.f};
    float far_plane{0.f};

    float tan_half_fov{0.f}; // Spot lights.
    float depth_scale{0.f};  // Ortho: 1/(f-n). Perspective: n*f/(f-n).
    glm::vec2 padding{0.f};
};
static_assert(sizeof(GpuShadow) == 64 + 16 * 3, "std140 layout");
static_assert(offsetof(GpuShadow, enabled) == 64);
static_assert(offsetof(GpuShadow, slope_bias_texels) == 80);
static_assert(offsetof(GpuShadow, tan_half_fov) == 96);

struct GpuShadowData {
    std::array<GpuShadow, k_max_lights> shadows{};
};
static_assert(sizeof(GpuShadowData) == 112 * k_max_lights,
              "std140 array layout");

// The depth map lives on its own texture unit: 0 is the albedo map.
inline constexpr uint32_t k_shadow_texture_unit = 2;

class ShadowUniforms {
  public:
    static constexpr uint32_t kBinding = 2;
    explicit ShadowUniforms(uint32_t binding = kBinding);
    void update(const GpuShadowData &shadow);

  private:
    Ref<UniformBuffer> buffer_;
    GpuShadowData data_{};
    uint32_t binding_{kBinding};
};
} // namespace ivx
