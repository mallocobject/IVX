#pragma once

#include "glm/ext.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/glm.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"

namespace ivx {

class OrthoGraphicCamera : public RefCounted {
  public:
    OrthoGraphicCamera(float left, float right, float bottom, float top);

    void set_position(const glm::vec3 &position) {
        position_ = position;
    }
    const glm::vec3 &get_position() const {
        return position_;
    }

    // radians
    void set_rotation(float degrees) {
        degrees_ = degrees;
    }
    float get_degrees() const {
        return degrees_;
    }

    void set_field(float left, float right, float bottom, float top) {
        left_ = left;
        right_ = right;
        bottom_ = bottom;
        top_ = top;
    }

    // you need save result, because it always calculate matrix per call this
    // function
    glm::mat4 get_view_matrix() const;
    glm::mat4 get_proj_matrix() const;

  private:
    float left_;
    float right_;
    float bottom_;
    float top_;

    glm::vec3 position_{0.f};
    float degrees_{0.f};
};
} // namespace ivx
