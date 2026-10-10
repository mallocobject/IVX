#include "ivxpch.h"
#include "timer.h"

namespace ivx {
float Timer::tick() {
    auto now = Clock::now();
    std::chrono::duration<float> dt = now - last_time_;
    last_time_ = now;
    return dt.count();
}

float Timer::peek() {
    auto now = Clock::now();
    return std::chrono::duration<float>(now - last_time_).count();
}

void Timer::reset() {
    last_time_ = Clock::now();
}
} // namespace ivx