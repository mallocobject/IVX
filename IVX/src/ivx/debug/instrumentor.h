#pragma once

#include "ivx/core/logger.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <ratio>
#include <source_location>
#include <string_view>
#include <thread>

namespace ivx {
using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

struct ProfileResult {
    std::string_view name;
    FloatingPointMicroseconds start;
    std::chrono::microseconds elapsed_time;
    std::thread::id thread_id;
};

struct InstrumentationSession {
    std::string name;
};

class Instrumentor {
  public:
    Instrumentor &operator=(Instrumentor &&) = delete;

    void begin_session(std::string_view name,
                       const std::filesystem::path &filepath);

    void end_session();
    void write_profile(const ProfileResult &result);

    static Instrumentor &get();

  private:
    Instrumentor() = default;
    ~Instrumentor();

    void write_header();
    void write_footer();
    void internal_end_session();

    std::mutex mtx_;
    std::unique_ptr<InstrumentationSession> current_session_;
    std::ofstream output_stream_;
};

class InstrumentationTimer {
  public:
    explicit InstrumentationTimer(std::string_view name);
    InstrumentationTimer(
        std::source_location loc = std::source_location::current());
    ~InstrumentationTimer();

    void stop();

  private:
    using Clock = std::chrono::high_resolution_clock;

    static std::string clean_function_name(const char *raw);

    std::string name_;
    Clock::time_point start_time_point_;
    bool stopped_;
};
} // namespace ivx

#define IVX_PROFILE 1

#if IVX_PROFILE
#define IVX_PROFILE_BEGIN_SESSION(name, filepath)                              \
    ::ivx::Instrumentor::get().begin_session(name, filepath)
#define IVX_PROFILE_END_SESSION() ::ivx::Instrumentor::get().end_session()

#define IVX_PROFILE_CONCAT_IMPL(a, b) a##b
#define IVX_PROFILE_CONCAT(a, b) IVX_PROFILE_CONCAT_IMPL(a, b)

#define IVX_PROFILE_SCOPE(name)                                                \
    ::ivx::InstrumentationTimer IVX_PROFILE_CONCAT(timer, __LINE__)(name)

#define IVX_PROFILE_FUNCTION()                                                 \
    ::ivx::InstrumentationTimer IVX_PROFILE_CONCAT(timer, __LINE__)
#else
#define IVX_PROFILE_BEGIN_SESSION(name, filepath)
#define IVX_PROFILE_END_SESSION()
#define IVX_PROFILE_SCOPE(name)
#define IVX_PROFILE_FUNCTION()
#endif