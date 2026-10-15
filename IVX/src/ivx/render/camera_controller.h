#pragma once

#include "camera.h"
#include "ivx/core/ref.hpp"
#include "ivx/event/event.h"

namespace ivx {
class CameraController : public RefCounted {
  public:
    CameraController(float aspect_ratio,
                     float zoom_level = 1.f,
                     bool enable_rotation = false);

    void on_update(float delta_time);
    void on_event(Event &event);

    Ref<OrthoGraphicCamera> get_camera() const {
        return camera_;
    }

  private:
    float aspect_ratio_;
    float zoom_level_;
    Ref<OrthoGraphicCamera> camera_;

    bool enable_rotation_;
};
} // namespace ivx