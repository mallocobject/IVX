#pragma once

#include <chrono>

namespace ivx {
class FrameTimer {
  public:
    // return delta time and update last time
    static float tick() {
        auto now = Clock::now();
        std::chrono::duration<float> dt = now - last_time_;
        last_time_ = now;
        return dt.count();
    }

    // return delta time but do not update last time
    static float peek() {
        auto now = Clock::now();
        return std::chrono::duration<float>(now - last_time_).count();
    }

    static void reset() {
        last_time_ = Clock::now();
    }

  private:
    using Clock = std::chrono::steady_clock;

    inline static Clock::time_point last_time_ = Clock::now();
};
} // namespace ivx
