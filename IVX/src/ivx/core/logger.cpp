#include "ivxpch.h"
#include "logger.h"

#include <cstdlib>
#include <mutex>
#include <utility>

namespace ivx {
namespace {
struct Settings {
    std::string dir; // empty -> console only
    std::string prefix = "ivx";
    LogFileFormat format = LogFileFormat::kCompressed;
    std::size_t max_file_size = 100ull * 1024 * 1024;
    int expire_days = 7;
    LogLevel threshold = LogLevel::VERBOSE;
};

std::mutex g_mutex;
Settings g_settings;

// bqlog level names; they are "verbose" and "warning", not "trace"/"warn".
constexpr const char *k_level_names[] = {
    "verbose", "debug", "info", "warning", "error", "fatal"};

constexpr int k_level_count = 6;

std::string level_list(LogLevel from) {
    std::string out = "[";
    for (int i = static_cast<int>(from); i < k_level_count; i++) {
        if (out.size() > 1) {
            out += ',';
        }
        out += k_level_names[i];
    }
    return out + "]";
}

std::string build_config(const char *file_suffix) {
    // thread_mode only takes effect at creation time.
    std::string cfg = "log.thread_mode=async\n";

    const auto levels = level_list(g_settings.threshold);

    cfg += "appenders_config.console.type=console\n";
    cfg += "appenders_config.console.levels=" + levels + "\n";

    if (!g_settings.dir.empty()) {
        cfg += "appenders_config.file.type=";
        cfg += g_settings.format == LogFileFormat::kText ? "text_file"
                                                         : "compressed_file";
        cfg += "\n";
        cfg += "appenders_config.file.levels=" + levels + "\n";
        cfg += "appenders_config.file.file_name=" + g_settings.dir + "/" +
               g_settings.prefix + file_suffix + "\n";
        cfg += "appenders_config.file.max_file_size=" +
               std::to_string(g_settings.max_file_size) + "\n";
        cfg += "appenders_config.file.expire_time_days=" +
               std::to_string(g_settings.expire_days) + "\n";
    }
    return cfg;
}

void apply_config_locked() {
    for (auto *slot : {&detail::g_core_slot, &detail::g_app_slot}) {
        if (auto *log = slot->ptr.load(std::memory_order_relaxed)) {
            log->reset_config(build_config(slot->file_suffix));
        }
    }
}
} // namespace

namespace detail {
LogSlot g_core_slot{"IVX", ""};
LogSlot g_app_slot{"APP", "_app"};

bq::log *make_log(LogSlot &slot) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (auto *existing = slot.ptr.load(std::memory_order_relaxed)) {
        return existing;
    }

    // Both objects are created together so a first call in either family does
    // not have to build the other one later.
    for (auto *s : {&g_core_slot, &g_app_slot}) {
        if (s->ptr.load(std::memory_order_relaxed) != nullptr) {
            continue;
        }

        // bq::log is a handle; the object lives in bqlog's global registry
        // until process exit, so the allocation is intentional. Async logging
        // needs an explicit flush before exiting.
        auto *created = new bq::log(
            bq::log::create_log(s->name, build_config(s->file_suffix)));
        s->ptr.store(created, std::memory_order_release);
    }

    std::atexit([] { bq::log::force_flush_all_logs(); });

    return slot.ptr.load(std::memory_order_relaxed);
}
} // namespace detail

void set_log_path(std::string dir,
                  std::string prefix,
                  LogFileFormat format,
                  std::size_t max_file_size,
                  int expire_days) {
    std::lock_guard<std::mutex> lock(g_mutex);

    g_settings.dir = std::move(dir);
    g_settings.prefix = prefix.empty() ? "ivx" : std::move(prefix);
    g_settings.format = format;
    g_settings.max_file_size = max_file_size;
    g_settings.expire_days = expire_days;

    // Flush first: entries already queued must not be dropped by the new level
    // filter or by an appender being recreated.
    bq::log::force_flush_all_logs();
    apply_config_locked();
}

void set_log_threshold(LogLevel lv) {
    std::lock_guard<std::mutex> lock(g_mutex);

    g_settings.threshold = lv;

    // Flush first so queued entries are emitted under the old threshold.
    bq::log::force_flush_all_logs();
    apply_config_locked();
}

void flush_logs() {
    // Covers both log objects; safe to call before they exist.
    bq::log::force_flush_all_logs();
}

namespace {
// Set for the duration of drain_console(). The bqlog API has no user-data
// parameter, so the sink is passed through this pointer instead. Only the main
// thread ever calls drain_console(), and the callback never logs, as required by
// bqlog's documentation.
std::vector<ConsoleMessage> *g_console_sink = nullptr;

void BQ_STDCALL console_buffer_callback(std::uint64_t log_id,
                                       std::int32_t /*category_idx*/,
                                       bq::log_level level,
                                       const char *content,
                                       std::int32_t length) {
    if (g_console_sink == nullptr) {
        return;
    }

    ConsoleMessage message;
    message.log_id = log_id;
    message.level = static_cast<LogLevel>(level); // same order as bq::log_level
    if (content != nullptr && length > 0) {
        message.text.assign(content, static_cast<std::size_t>(length));
    }
    g_console_sink->push_back(std::move(message));
}
} // namespace

void enable_console_capture(bool enable) {
    bq::log::set_console_buffer_enable(enable);
}

void drain_console(std::vector<ConsoleMessage> &out) {
    g_console_sink = &out;
    // The API pops a single entry per call and returns false once drained.
    while (bq::log::fetch_and_remove_console_buffer(&console_buffer_callback)) {
    }
    g_console_sink = nullptr;
}
} // namespace ivx
