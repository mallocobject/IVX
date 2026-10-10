#pragma once

#include <chrono>

namespace ivx {
class Timer {
  public:
    // return delta time and update last time
    static float tick();

    // return delta time but do not update last time
    static float peek();

    static void reset();

  private:
    using Clock = std::chrono::steady_clock;

    inline static Clock::time_point last_time_ = Clock::now();
};
} // namespace ivx