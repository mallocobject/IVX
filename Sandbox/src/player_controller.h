#pragma once

#include "ivx/scene/scriptable_entity.h"

namespace ivx {

// Minimal native script: moves the owning entity with WASD and spins it with
// Q/E.
class PlayerController : public ScriptableEntity {
  public:
    void on_create() override;
    void on_update(float delta_time) override;

  private:
    float move_speed_{200.f}; // pixels per second
    float turn_speed_{90.f};  // degrees per second
};

} // namespace ivx
