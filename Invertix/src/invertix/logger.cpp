#include "ivx_pch.h"
#include "invertix/logger.h"
#include <elog/logger.hpp>


namespace invertix {
	void set_log_threshold(LogLevel lv) {
		elog::set_log_threshold(static_cast<elog::LogLevel>(lv));
	}
	void log_message(LogLevel lv, std::string_view who, std::string_view msg) {
		elog::details::output_log(static_cast<elog::LogLevel>(lv), who, msg, std::source_location::current());
	}
	void set_log_path(const std::string& dir,
		const std::string& prefix,
		size_t roll_size,
		std::chrono::seconds flush_interval,
		size_t check_per_count) {
		elog::set_log_path(dir, prefix, roll_size, flush_interval, check_per_count);
	}
}