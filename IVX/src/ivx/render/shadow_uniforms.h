#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/uniform_buffer.h"

#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {
struct GpuShadow {
    glm::mat4 light_space{1.f}; // world -> light clip
    // x enabled, y minimum bias in projected depth, z world units per texel,
    // w normal offset in texels. UV texel size comes from textureSize().
    glm::vec4 params{0.f};
    // x angle-dependent bias in projected depth; independent of screen pixels.
    glm::vec4 params2{0.f};
    glm::ivec4 light_index{0};
};
static_assert(sizeof(GpuShadow) == 64 + 16 + 16 + 16, "std140 layout");

// The depth map lives on its own texture unit: 0 is the albedo map.
inline constexpr uint32_t k_shadow_texture_unit = 2;

class ShadowUniforms {
  public:
    static constexpr uint32_t kBinding = 2;
    explicit ShadowUniforms(uint32_t binding = kBinding);
    void update(const GpuShadow &shadow);

  private:
    Ref<UniformBuffer> buffer_;
    GpuShadow data_{};
    uint32_t binding_{kBinding};
};
} // namespace ivx
