#pragma once

#include <chrono>
#include <concepts>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ivx {
struct ProfileResult {
    float duration;
    std::string_view label;
};

template <typename Fn>
    requires std::invocable<std::decay_t<Fn> &, ProfileResult>
class ScopedTimer {
  public:
    ScopedTimer(std::string_view label, Fn &&func)
        : label_(label), func_(std::forward<Fn>(func)),
          start_time_point_(Clock::now()), stopped_(false) {
    }

    ~ScopedTimer() {
        stop();
    }

    ScopedTimer &operator=(ScopedTimer &&) = delete;

    void stop() {
        if (stopped_) {
            return;
        }
        stopped_ = true;

        auto end_time_point = Clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time_point - start_time_point_);
        float milliseconds = static_cast<float>(us.count()) * 0.001f;
        func_({milliseconds, label_});
        stopped_ = true;
    }

  private:
    using Clock = std::chrono::high_resolution_clock;

    std::string_view label_;
    std::decay_t<Fn> func_;
    Clock::time_point start_time_point_;

    bool stopped_;
};

} // namespace ivx

#define PROFILE_SCOPE(label)                                                   \
    ivx::ScopedTimer timer##__FILE__##__LINE__(                                \
        label, [&prs = prs_](ivx::ProfileResult pr) {                          \
            prs.push_back(std::move(pr));                                      \
        })
