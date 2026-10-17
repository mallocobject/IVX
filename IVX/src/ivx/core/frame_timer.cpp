#include "ivxpch.h"
#include "frame_timer.h"

namespace ivx {
float FrameTimer::tick() {
    auto now = Clock::now();
    std::chrono::duration<float> dt = now - last_time_;
    last_time_ = now;
    return dt.count();
}

float FrameTimer::peek() {
    auto now = Clock::now();
    return std::chrono::duration<float>(now - last_time_).count();
}

void FrameTimer::reset() {
    last_time_ = Clock::now();
}
} // namespace ivx