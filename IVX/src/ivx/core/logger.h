#pragma once

#include "ivx/core/base.h"
#include <atomic>
#include <bq_log/bq_log.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ivx {

enum class LogLevel : std::uint8_t { VERBOSE, DEBUG, INFO, WARN, ERROR, FATAL };

// Serialisation of the file appender.
enum class LogFileFormat : std::uint8_t {
    kCompressed, // ".logcompr": written continuously, needs BqLog_LogDecoder
    kText,       // ".log": plain utf-8, but bqlog only flushes it on clean exit
};

// Turns file output on. Never calling it (or passing an empty dir) keeps the
// console appender only. The two log objects write separate files:
// "<dir>/<prefix>_*.logcompr" (IVX) and "<dir>/<prefix>_app_*.logcompr" (APP).
void set_log_path(std::string dir,
                  std::string prefix = "ivx",
                  LogFileFormat format = LogFileFormat::kCompressed,
                  std::size_t max_file_size = 100ull * 1024 * 1024,
                  int expire_days = 7);

// Drops everything below `lv` inside bqlog, before any formatting or buffering
// happens, so filtered call sites stay cheap. Applied to both log objects.
void set_log_threshold(LogLevel lv);

// Blocks until buffered entries reach the appenders (also runs at exit).
void flush_logs();

// ---- in-editor console capture ----
// While enabled, bqlog stores console output in its own thread-safe buffer
// instead of invoking callbacks from the logging threads. drain_console() then
// pulls the entries on the main thread, which is what an ImGui console needs.
// Off by default: an app that never drains would grow the buffer forever.
void enable_console_capture(bool enable);

struct ConsoleMessage {
    std::uint64_t log_id{0}; // 0 = engine log object, see bqlog's registry
    LogLevel level{LogLevel::INFO};
    std::string text;
};

// Moves every buffered message into `out`. Main thread only.
void drain_console(std::vector<ConsoleMessage> &out);

namespace detail {
// "IVX" (engine) and "APP" (client) are two bqlog log objects, which is what
// makes the tag free: bqlog prints the log name as the "[Bq][IVX]" prefix, so
// nothing has to be concatenated into the message.
struct LogSlot {
    std::atomic<bq::log *> ptr{nullptr};
    const char *name;        // shown by bqlog as "[Bq][<name>]"
    const char *file_suffix; // appended to set_log_path's prefix

    LogSlot(const char *n, const char *suffix) : name(n), file_suffix(suffix) {
    }
};

extern LogSlot g_core_slot;
extern LogSlot g_app_slot;

bq::log *make_log(LogSlot &slot); // cold path: creates both objects at once

inline bq::log &log_ref(LogSlot &slot) {
    if (auto *p = slot.ptr.load(std::memory_order_acquire); p != nullptr) {
        return *p;
    }
    return *make_log(slot);
}

inline bq::log &core_log() {
    return log_ref(g_core_slot);
}

inline bq::log &app_log() {
    return log_ref(g_app_slot);
}
} // namespace detail
} // namespace ivx

// Zero-cost wrapper: a call site compiles down to a single bqlog call. The
// engine/client distinction is carried by the log object itself (its name shows
// up as the "[Bq][IVX]" / "[Bq][APP]" prefix), so the format string is passed
// through untouched -- no literal-only restriction, no allocation, no
// std::format. bqlog tests the level bitmap first and returns before touching
// the arguments, which is what makes the threshold free.
#define IVX_CORE_VERBOSE(...) (::ivx::detail::core_log().verbose(__VA_ARGS__))
#define IVX_CORE_DEBUG(...) (::ivx::detail::core_log().debug(__VA_ARGS__))
#define IVX_CORE_INFO(...) (::ivx::detail::core_log().info(__VA_ARGS__))
#define IVX_CORE_WARN(...) (::ivx::detail::core_log().warning(__VA_ARGS__))
#define IVX_CORE_ERROR(...) (::ivx::detail::core_log().error(__VA_ARGS__))
#define IVX_CORE_FATAL(...) (::ivx::detail::core_log().fatal(__VA_ARGS__))

#define IVX_VERBOSE(...) (::ivx::detail::app_log().verbose(__VA_ARGS__))
#define IVX_DEBUG(...) (::ivx::detail::app_log().debug(__VA_ARGS__))
#define IVX_INFO(...) (::ivx::detail::app_log().info(__VA_ARGS__))
#define IVX_WARN(...) (::ivx::detail::app_log().warning(__VA_ARGS__))
#define IVX_ERROR(...) (::ivx::detail::app_log().error(__VA_ARGS__))
#define IVX_FATAL(...) (::ivx::detail::app_log().fatal(__VA_ARGS__))
