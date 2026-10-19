#include "ivxpch.h"
#include "instrumentor.h"
#include "ivx/core/logger.h"
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace ivx {
void Instrumentor::begin_session(std::string_view name,
                                 const std::filesystem::path &filepath) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (current_session_) {
        IVX_CORE_ERROR(
            "Instrumentor::begin_session('{}') when session '{}' already open.",
            name,
            current_session_->name);
        internal_end_session();
    }

    output_stream_.open(filepath.c_str());
    if (output_stream_.is_open()) {
        current_session_ =
            std::make_unique<InstrumentationSession>(std::string{name});
        write_header();
    } else {
        IVX_CORE_ERROR("Instrumentor could not open results file '{}'.",
                       filepath.string());
    }
}

void Instrumentor::end_session() {
    std::lock_guard<std::mutex> lcok(mtx_);
    internal_end_session();
}

void Instrumentor::write_profile(const ProfileResult &result) {
    std::string json = std::format(
        R"(,{{"cat":"function","dur":{},"name":"{}","ph":"X","pid":0,"tid":{},"ts":{:.3f}}})",
        result.elapsed_time.count(),
        result.name,
        std::hash<std::thread::id>{}(result.thread_id),
        result.start.count());

    std::lock_guard<std::mutex> lock(mtx_);
    if (current_session_) {
        output_stream_ << json;
        output_stream_.flush();
    }
}

Instrumentor &Instrumentor::get() {
    static Instrumentor instance;
    return instance;
}

Instrumentor::~Instrumentor() {
    end_session();
}

void Instrumentor::write_header() {
    output_stream_ << R"({"otherData": {},"traceEvents":[{})";
    output_stream_.flush();
}

void Instrumentor::write_footer() {
    output_stream_ << "]}";
    output_stream_.flush();
}

void Instrumentor::internal_end_session() {
    if (current_session_) {
        write_footer();
        output_stream_.close();
        current_session_.reset();
    }
}

InstrumentationTimer::InstrumentationTimer(std::string_view name)
    : name_(name), start_time_point_(Clock::now()), stopped_(false) {
}

InstrumentationTimer::InstrumentationTimer(std::source_location loc)
    : InstrumentationTimer((clean_function_name(loc.function_name()))) {
}

InstrumentationTimer::~InstrumentationTimer() {
    if (!stopped_) {
        stop();
    }
}

void InstrumentationTimer::stop() {
    auto end_time_point = Clock::now();
    auto high_res_start =
        FloatingPointMicroseconds{start_time_point_.time_since_epoch()};
    auto elapsed_time =
        std::chrono::time_point_cast<std::chrono::microseconds>(end_time_point)
            .time_since_epoch() -
        std::chrono::time_point_cast<std::chrono::microseconds>(
            start_time_point_)
            .time_since_epoch();

    Instrumentor::get().write_profile(
        {name_, high_res_start, elapsed_time, std::this_thread::get_id()});

    stopped_ = true;
}

std::string InstrumentationTimer::clean_function_name(const char *raw) {
    std::string_view sv{raw};
    if (auto pos = sv.find("__cdecl "); pos != std::string_view::npos) {
        sv.remove_prefix(pos + 8);
    }

    std::string result{sv};
    rg::replace(result, '"', '\'');
    return result;
}

} // namespace ivx