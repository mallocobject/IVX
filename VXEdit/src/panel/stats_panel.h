#pragma once

#include "panel.h"

#include <string>
#include <vector>

namespace ivx {

// Development-only timing panel: whole-frame cost plus the phase breakdown the
// layer reports every frame (update / render), a frame-time graph and the
// Renderer2D counters. Not meant to survive into a shipped editor.
//
// Layout: numbers on the left, frame-time graph on the right.
//
// It is a floating overlay in the top-right corner rather than a docked panel:
// it is reference information you glance at, and a dock slot would compete with
// the console for the bottom strip.
class StatsPanel : public Panel {
  public:
    StatsPanel();

    // 60 Hz budget; the graph and the headline are coloured against it.
    static constexpr float kBudgetMs = 16.7f;

    // Called once per frame with the frame delta; finalizes the previous
    // frame's measurements and pushes the new sample into the history.
    void begin_frame(float delta_time);

    // The overlay only makes sense while the viewport is on screen: hide it
    // when the viewport tab is not the selected one (e.g. while writing a
    // script).
    void set_host_visible(bool visible) {
        host_visible_ = visible;
    }

    // Per-phase measurement, milliseconds. Collected until the next frame.
    void submit(const char *phase, float milliseconds);

  protected:
    void on_imgui() override;
    // Pins the window to the top-right corner of the main viewport.
    bool begin_window() override;
    // Keeps the overlay above the docked panels.
    void end_window() override;
    bool should_draw() const override {
        return opened_ && host_visible_;
    }

  private:
    struct Phase {
        std::string name;
        float ms{0.f};
    };

    void draw_info_column();
    void draw_graph_column();

    static constexpr int kHistorySize = 240;

    std::vector<Phase> current_;
    std::vector<Phase> reported_;

    float history_[kHistorySize]{};
    int history_offset_{0};
    int history_count_{0};

    // True once the overlay has been placed inside the viewport for the first
    // time.
    bool placed_{false};
    bool host_visible_{true};
    // Scale the window was last auto-sized for (see begin_window).
    float sized_for_scale_{0.f};

    float frame_ms_{0.f}; // raw delta of the finalized frame
    float smoothed_ms_{0.f};
    float min_ms_{0.f};
    float max_ms_{0.f};
    float avg_ms_{0.f};
};

} // namespace ivx
