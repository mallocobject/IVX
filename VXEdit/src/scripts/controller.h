#pragma once
#include "ivx/scene/scriptable_entity.h"
#include <ivx.h>

namespace ivx {
class Controller : public ScriptableEntity {
  public:
    virtual void on_create() override;
    virtual void on_update(float dt) override;
    virtual void on_destroy() override;
};
} // namespace ivx