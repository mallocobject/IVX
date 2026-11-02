#include "ivxpch.h"
#include "light_uniforms.h"

#include <algorithm>
#include <cstring>

namespace ivx {

LightUniforms::LightUniforms(uint32_t binding) : binding_(binding) {
    buffer_ = UniformBuffer::create(sizeof(GpuLightData), binding_);
}

void LightUniforms::update(const LightSet &lights) {
    GpuLightData next;
    next.ambient_exposure = glm::vec4{lights.ambient, lights.exposure};
    next.counts.x = static_cast<int>(std::min(lights.count, k_max_lights));

    for (uint32_t i = 0; i < static_cast<uint32_t>(next.counts.x); ++i) {
        const Light &light = lights.lights[i];
        const LightSettings &settings = light.settings;
        GpuLight &gpu = next.lights[i];
        gpu.position_type =
            glm::vec4{light.position, static_cast<float>(settings.type)};
        gpu.direction_linear = glm::vec4{light.direction, settings.linear};
        gpu.color_intensity = glm::vec4{settings.color, settings.intensity};
        gpu.falloff = glm::vec4{
            settings.constant, settings.linear, settings.quadratic, 0.f};
        gpu.spot_angles = glm::vec4{light.inner_cos, light.outer_cos, 0.f, 0.f};
    }

    if (std::memcmp(&next, &data_, sizeof(GpuLightData)) != 0) {
        data_ = next;
        buffer_->set_data(&data_, sizeof(GpuLightData));
    }
    buffer_->bind(binding_);
}

} // namespace ivx