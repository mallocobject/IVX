#pragma once

#include "panel.h"

#include <string>
#include <vector>

namespace ivx {

// Development-only timing panel: whole-frame cost plus the phase breakdown the
// layer reports every frame (update / render), a frame-time graph and the
// Renderer2D counters. Not meant to survive into a shipped editor.
class StatsPanel : public Panel {
  public:
    StatsPanel();

    // Called once per frame with the frame delta; finalizes the previous frame's
    // measurements and pushes the new sample into the history.
    void begin_frame(float delta_time);

    // Per-phase measurement, milliseconds. Collected until the next frame.
    void submit(const char *phase, float milliseconds);

  protected:
    void on_imgui() override;

  private:
    struct Phase {
        std::string name;
        float ms{0.f};
    };

    static constexpr int kHistorySize = 240;

    std::vector<Phase> current_;
    std::vector<Phase> reported_;

    float history_[kHistorySize]{};
    int history_offset_{0};
    int history_count_{0};

    float frame_ms_{0.f}; // raw delta of the finalized frame
    float smoothed_ms_{0.f};
    float min_ms_{0.f};
    float max_ms_{0.f};
};

} // namespace ivx
