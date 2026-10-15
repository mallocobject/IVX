#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/event/event.h"
#include "ivx/renderer/camera.h"

namespace ivx {
class OrthoGraphicCameraController : public RefCounted {
  public:
    OrthoGraphicCameraController(int width,
                                 int hegiht,
                                 float zoom_level = 1.f,
                                 bool enable_rotation = false);

    void on_update(float delta_time);
    void on_event(Event &event);

    Ref<OrthoGraphicCamera> get_camera() const {
        return camera_;
    }

  private:
    int width_;
    int height_;
    float zoom_level_;
    Ref<OrthoGraphicCamera> camera_;

    bool enable_rotation_;
};

class PerspectiveCameraController : public RefCounted {
  public:
    PerspectiveCameraController(const glm::vec3 &eye,
                                const glm::vec3 &center,
                                const glm::vec3 &up,
                                float fov,
                                int width,
                                int height,
                                float near,
                                float far);

    void on_update(float delta_time);
    void on_event(Event &event);

    Ref<PerspectiveCamera> get_camera() const {
        return camera_;
    }

  private:
    Ref<PerspectiveCamera> camera_;
};
} // namespace ivx
