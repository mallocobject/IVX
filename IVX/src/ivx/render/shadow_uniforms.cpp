#include "ivxpch.h"
#include "shadow_uniforms.h"

#include <cstring>

namespace ivx {
ShadowUniforms::ShadowUniforms(uint32_t binding) : binding_(binding) {
    buffer_ = UniformBuffer::create(sizeof(GpuShadowData), binding_);
    buffer_->set_data(&data_, sizeof(data_));
}

void ShadowUniforms::update(const GpuShadowData &shadow) {
    if (std::memcmp(&shadow, &data_, sizeof(GpuShadowData)) != 0) {
        data_ = shadow;
        buffer_->set_data(&data_, sizeof(GpuShadowData));
    }
    buffer_->bind(binding_);
}
} // namespace ivx
