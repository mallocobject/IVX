#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/event/event.h"
#include "ivx/renderer/camera.h"

namespace ivx {
class CameraController : public RefCounted {
  public:
    CameraController(int width,
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
} // namespace ivx
