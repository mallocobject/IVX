#include "ivxpch.h"
#include "shadow_uniforms.h"

#include <cstring>

namespace ivx {
ShadowUniforms::ShadowUniforms(uint32_t binding) : binding_(binding) {
    buffer_ = UniformBuffer::create(sizeof(GpuShadow), binding_);
    buffer_->set_data(&data_, sizeof(data_));
}

void ShadowUniforms::update(const GpuShadow &shadow) {
    if (std::memcmp(&shadow, &data_, sizeof(GpuShadow)) != 0) {
        data_ = shadow;
        buffer_->set_data(&data_, sizeof(GpuShadow));
    }
    buffer_->bind(binding_);
}
} // namespace ivx
