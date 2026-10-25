#pragma once

#include "ivx/scene/scriptable_entity.h"

namespace ivx {

// Moves its own entity with WASD (Q/E rotate) and zooms with the scroll wheel.
// Lives in the VXScripts module, so it can be rebuilt and hot reloaded without
// touching the editor.
//
// Movement is polled in on_update() (continuous input), while discrete input
// arrives through the event hooks below -- that is the split the engine offers.
class CameraController : public ScriptableEntity {
  public:
    void on_create() override;
    void on_update(float dt) override;
    void on_destroy() override;

    void on_key_pressed(KeyCode key, bool repeat) override;
    void on_mouse_scrolled(float x_offset, float y_offset) override;
};
} // namespace ivx
