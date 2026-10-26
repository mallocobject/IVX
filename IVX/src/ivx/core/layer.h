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

    // UI phase: builds this layer's ImGui windows.
    //
    // Application calls it between ImGuiLayer::begin()/end(), i.e. after the
    // frame's GPU work is done. Emitting render commands here would corrupt
    // ImGui's state, so the only draw calls allowed are ImGui's own.
    //
    // There is deliberately no on_render() hook (Hazel names this
    // OnImGuiRender): rendering belongs to on_update(), where a layer still
    // owns the GL state and picks its own render target (the editor draws into
    // the viewport framebuffer, a game draws into the swap chain).
    virtual void on_imgui() {};

    inline std::string_view get_name() const {
        return debug_name_;
    }

  protected:
    std::string debug_name_;
};

} // namespace ivx
