#pragma once

#include "camera.h"
#include "ivx/core/ref.hpp"
#include "uniform_buffer.h"
#include <glm/glm.hpp>

namespace ivx {

// std140 mirror of `struct Camera` in the shaders' CameraBlock (208 bytes).
struct CameraData {
    glm::mat4 view{1.f};
    glm::mat4 projection{1.f};
    glm::mat4 view_projection{1.f};
    glm::vec4 position{0.f, 0.f, 0.f, 0.f}; // xyz
};
static_assert(sizeof(CameraData) == 208,
              "CameraData must match the std140 layout");

// One per pass: the active instance owns the binding point until another pass
// claims it.
class CameraUniforms {
  public:
    static constexpr uint32_t kBinding = 0;

    explicit CameraUniforms(uint32_t binding = kBinding);
    ~CameraUniforms() = default;

    CameraUniforms(const CameraUniforms &) = delete;
    CameraUniforms &operator=(const CameraUniforms &) = delete;

    // Uploads (skipped when unchanged) and binds; call at the start of the
    // pass.
    void update(const Camera &camera);
    void bind();

    const CameraData &get_data() const {
        return data_;
    }

  private:
    Ref<UniformBuffer> buffer_;
    CameraData data_{};
    uint32_t binding_{kBinding};
};
} // namespace ivx
