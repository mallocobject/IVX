#pragma once

#include "ivx/core/layer.h"

#include <string>

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

    // Where ImGui stores the window layout (docking, panel sizes, ...). An empty
    // string disables persistence (io.IniFilename = nullptr). Setting it after
    // on_attach() takes effect immediately.
    void set_ini_filename(std::string filename);

  private:
    bool event_blocked_{true};
    std::string ini_filename_;
};
} // namespace ivx
