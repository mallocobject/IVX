#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/light.h"
#include "uniform_buffer.h"

#include <array>
#include <glm/glm.hpp>

namespace ivx {

struct GpuLight {
    glm::vec4 position_type{0.f};
    glm::vec4 direction_range{0.f, -1.f, 0.f, 1000.f};
    glm::vec4 color_intensity{1.f};
    glm::vec4 spot_angles{1.f, 0.f, 0.f, 0.f};
};

// std140 mirror of the LightBlock in mesh.glsl.
struct GpuLightData {
    glm::vec4 ambient_exposure{0.f, 0.f, 0.f, 1.f};
    glm::ivec4 counts{0, 0, 0, 0};
    std::array<GpuLight, k_max_lights> lights{};
};
static_assert(sizeof(GpuLightData) == 16 + 16 + k_max_lights * 64,
              "GpuLightData must match the std140 layout");

class LightUniforms {
  public:
    static constexpr uint32_t kBinding = 1;

    explicit LightUniforms(uint32_t binding = kBinding);
    ~LightUniforms() = default;

    LightUniforms(const LightUniforms &) = delete;
    LightUniforms &operator=(const LightUniforms &) = delete;

    // Uploads (skipped when unchanged) and claims the binding point.
    void update(const LightSet &lights);

  private:
    Ref<UniformBuffer> buffer_;
    GpuLightData data_{};
    uint32_t binding_{kBinding};
};

} // namespace ivx