#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {
class SceneCamera {
  public:
    // orthogonal
    SceneCamera(float size, float n, float f, float zoom_level);

    // perspective
    SceneCamera(float fov, float n, float f);

    void set_viewport_size(uint32_t width, uint32_t height);

    void set_zoom_level(float zoom_level);
    void set_fov(float fov);

    glm::mat4 get_proj_matrix() const;

    enum class Mode { Orthogonal, Perspective } mode;

  private:
    union {
        struct {
            float size;
            float zoom;
        } size_zoom_;
        float fov_;
    };

    float n_;
    float f_;

    float aspect_ratio_{1.f};
};
} // namespace ivx