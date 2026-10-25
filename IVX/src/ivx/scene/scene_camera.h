#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {
class SceneCamera {
  public:
    enum class Mode { Orthogonal, Perspective };

    // orthogonal
    SceneCamera(float size, float n, float f, float zoom_level);

    // perspective
    SceneCamera(float fov, float n, float f);

    void set_viewport_size(uint32_t width, uint32_t height);

    void set_zoom_level(float zoom_level);
    void set_fov(float fov);

    // Inspectors need to read back what they can edit.
    void set_mode(Mode m);
    Mode get_mode() const {
        return mode;
    }

    float get_size() const; // orthogonal only
    void set_size(float size);

    float get_zoom_level() const; // orthogonal only

    float get_fov() const; // perspective only

    float get_near() const;
    void set_near(float n);
    float get_far() const;
    void set_far(float f);

    glm::mat4 get_proj_matrix() const;

    Mode mode;

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
