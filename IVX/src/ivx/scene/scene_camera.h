#pragma once

#include <glm/glm.hpp>

namespace ivx {
class SceneCamera {
  public:
    SceneCamera(float left,
                float right,
                float bottom,
                float top,
                float n,
                float f,
                float zoom_level);

    void set_view_field(
        float left, float right, float bottom, float top, float n, float f);

    void set_zoom_level(float zoom_level);

    glm::mat4 get_proj_matrix() const;

  private:
    float left_;
    float right_;
    float bottom_;
    float top_;
    float n_;
    float f_;
    float zoom_level_;
};
} // namespace ivx