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

  private:
    float time_{0.f};
};
} // namespace ivx
