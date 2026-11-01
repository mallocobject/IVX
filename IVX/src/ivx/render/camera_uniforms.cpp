#include "ivxpch.h"
#include "camera_uniforms.h"

#include <cstring>

namespace ivx {

CameraUniforms::CameraUniforms(uint32_t binding) : binding_(binding) {
    buffer_ = UniformBuffer::create(sizeof(CameraData), binding_);
}

void CameraUniforms::update(const Camera &camera) {
    CameraData next;
    next.view = camera.get_view_matrix();
    next.projection = camera.get_proj_matrix();
    next.view_projection = next.projection * next.view;
    next.position = glm::vec4{camera.get_position(), 0.f};

    if (std::memcmp(&next, &data_, sizeof(CameraData)) != 0) {
        data_ = next;
        buffer_->set_data(&data_, sizeof(CameraData));
    }

    // Another pass may have claimed the binding point since the last frame, so
    // claim it again even when the data did not change.
    buffer_->bind(binding_);
}

void CameraUniforms::bind() {
    buffer_->bind(binding_);
}

} // namespace ivx
