#pragma once

#include "ivx/core/layer.h"

namespace ivx {
class ImGuiLayer : public Layer {
  public:
    ImGuiLayer();
    ~ImGuiLayer() = default;

    virtual void on_attach() override;
    virtual void on_detach() override;
    virtual void on_event(Event &event) override;

    void begin();
    void end();

    void block_event(bool enabled = true) {
        event_blocked_ = enabled;
    }

  private:
    float time_{0.f};

    bool event_blocked_{true};
};
} // namespace ivx
