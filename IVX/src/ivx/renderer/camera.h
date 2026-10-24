#pragma once

#include "glm/ext.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/glm.hpp"
#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"

namespace ivx {
class Camera : public RefCounted {
  public:
    virtual ~Camera() = default;
    virtual glm::mat4 get_view_matrix() const = 0;
    virtual glm::mat4 get_proj_matrix() const = 0;
};

class OrthoGraphicCamera : public Camera {
  public:
    OrthoGraphicCamera(float left, float right, float bottom, float top);
    // ~OrthoGraphicCamera() = default;

    void set_position(const glm::vec3 &position) {
        position_ = position;
    }
    const glm::vec3 &get_position() const {
        return position_;
    }

    // radians
    void set_rotation(float rotation) {
        rotation_ = rotation;
    }
    float get_rotation() const {
        return rotation_;
    }

    void set_view_field(float left, float right, float bottom, float top) {
        left_ = left;
        right_ = right;
        bottom_ = bottom;
        top_ = top;
    }

    // you need save result, because it always calculate matrix per call this
    // function
    glm::mat4 get_view_matrix() const override;
    glm::mat4 get_proj_matrix() const override;

  private:
    float left_;
    float right_;
    float bottom_;
    float top_;

    glm::vec3 position_{0.f};
    float rotation_{0.f};
};

class PerspectiveCamera : public Camera {
  public:
    PerspectiveCamera(const glm::vec3 &eye,
                      const glm::vec3 &center,
                      const glm::vec3 &up,
                      float fov,
                      int width,
                      int height,
                      float n,
                      float f);
    // ~PerspectiveCamera() = default;

    void set_eye(const glm::vec3 &eye) {
        eye_ = eye;
    }

    const glm::vec3 &get_eye() const {
        return eye_;
    }

    void set_center(const glm::vec3 &center) {
        center_ = center;
    }

    const glm::vec3 &get_center() const {
        return center_;
    }

    void set_up(const glm::vec3 &up) {
        up_ = up;
    }

    const glm::vec3 &get_up() const {
        return up_;
    }

    void set_fov(float fov) {
        fov_ = fov;
    }

    float get_fov() const {
        return fov_;
    }

    void set_view_field(int width, int height) {
        width_ = width;
        height_ = height;
    }

    // you need save result, because it always calculate matrix per call this
    // function
    glm::mat4 get_view_matrix() const;
    glm::mat4 get_proj_matrix() const;

  private:
    glm::vec3 eye_;
    glm::vec3 center_;
    glm::vec3 up_;
    float fov_;
    int width_;
    int height_;
    float near_;
    float far_;
};
} // namespace ivx
