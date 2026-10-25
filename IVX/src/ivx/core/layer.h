#pragma once

#include "ivx/core/base.h"
#include "ivx/core/ref_counted.hpp"
#include "ivx/event/event.h"

namespace ivx {
class Layer : public RefCounted {
  public:
    Layer(const std::string &name = "Layer");
    virtual ~Layer() = default;

    virtual void on_attach() {
    }
    virtual void on_detach() {
    }
    virtual void on_update(float dt) {
    }
    virtual void on_event(Event &event) {
    }

    // for imgui
    virtual void on_render() {};

    inline std::string_view get_name() const {
        return debug_name_;
    }

  protected:
    std::string debug_name_;
};

} // namespace ivx
