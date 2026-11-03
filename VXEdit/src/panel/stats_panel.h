#pragma once

#include "panel.h"

#include <string>
#include <vector>

namespace ivx {

// Development-only timing panel: whole-frame cost plus the phase breakdown the
// layer reports every frame (update / render), a frame-time graph and the
// renderer counters.
//
// It is a floating OS window rather than a docked panel: the numbers are
// reference information you glance at on a second screen, and a dock slot would
// compete with the console for the bottom strip. With multi-viewport enabled
// the first placement puts it outside the main window, which is what makes it a
// window of its own.
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
    // Places the window once, outside the main window when the monitor has
    // room.
    bool begin_window() override;
    bool should_draw() const override {
        return opened_ && host_visible_;
    }

  private:
    struct Phase {
        std::string name;
        float ms{0.f};
    };

    // Where the window lands the first time: outside the main window when the
    // monitor has room, otherwise over its bottom-right corner.
    static ImVec2 detached_position(ImVec2 size);

    void draw_headline();
    void draw_info_column();
    void draw_graph_column();

    static constexpr int kHistorySize = 240;

    std::vector<Phase> current_;
    std::vector<Phase> reported_;

    float history_[kHistorySize]{};
    int history_offset_{0};
    int history_count_{0};

    // True once the window has been placed for the first time; from then on the
    // user's own position sticks.
    bool placed_{false};
    // False until the first begin_frame, whose delta is start-up time.
    bool started_{false};
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
