#pragma once
#include "ivx/scene/scriptable_entity.h"
#include <ivx.h>

namespace ivx {

// Moves its own entity with WASD (Q/E rotate). Registered as an available
// script; no entity has it bound by default, the editor binds it manually.
class CameraController : public ScriptableEntity {
  public:
    virtual void on_create() override;
    virtual void on_update(float dt) override;
    virtual void on_destroy() override;
};
} // namespace ivx
