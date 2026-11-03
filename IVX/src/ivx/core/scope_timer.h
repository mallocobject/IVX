#pragma once

#include <chrono>

namespace ivx {

// Scope timer for a phase inside one frame.
//
// FrameTimer measures the distance between two frames; this measures one span
// within a frame, and any number of them can be alive at once. The result is
// written into the float passed to the constructor, so the caller keeps flat
// code (no begin/end bookkeeping):
//
//     float update_ms = 0.f;
//     ScopeTimer timer{update_ms};
//     ...work...
//     timer.stop();
//     stats_panel_->submit("update", update_ms);
class ScopeTimer {
  public:
    explicit ScopeTimer(float &milliseconds) : out_(milliseconds) {
    }

    ~ScopeTimer() {
        stop();
    }

    ScopeTimer(const ScopeTimer &) = delete;
    ScopeTimer &operator=(const ScopeTimer &) = delete;

    // Writes the elapsed time and returns it. Idempotent: the destructor after
    // an explicit stop() does nothing.
    float stop() {
        if (stopped_) {
            return out_;
        }

        stopped_ = true;
        const Clock::time_point now = Clock::now();
        out_ = std::chrono::duration<float, std::milli>(now - start_).count();
        return out_;
    }

    // Elapsed so far, without stopping. Does not touch the output.
    float elapsed() const {
        if (stopped_) {
            return out_;
        }

        const Clock::time_point now = Clock::now();
        return std::chrono::duration<float, std::milli>(now - start_).count();
    }

  private:
    using Clock = std::chrono::steady_clock;

    Clock::time_point start_{Clock::now()};
    float &out_;
    bool stopped_{false};
};

} // namespace ivx
