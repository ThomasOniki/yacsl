#pragma once

#include "base/containers/char_buffer.hpp"
#include "base/containers/string_view.hpp"
#include "base/files/common.hpp"
#include "base/files/file.hpp"
#include "base/logging/common.hpp"
#include "base/logging/format_string.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class FileLogger {
public:
	explicit FileLogger(const StringView& file_path)
			: file_(file_path, FileOpenFlags::Write | FileOpenFlags::Create | FileOpenFlags::Append) {}

	explicit FileLogger(const StringView& file_path, LogLevel level)
			: file_(file_path, FileOpenFlags::Write | FileOpenFlags::Create | FileOpenFlags::Append),
				level_(level) {}

	template <typename... Args>
	void log(LogLevel lvl, const FormatString<Args...>& fmt, Args&&... args) {
		if (lvl < level_) {
			return;
		}
		CharBuffer buf;
		buf.append(LOG_LEVEL_STRINGS[static_cast<u8>(lvl)]);
		format_to(buf, fmt, base::forward<Args>(args)...);
		buf.append('\n');
		file_.write(StringView(buf.begin(), buf.end()));
	}

	void log(LogLevel lvl, const StringView& msg) {
		if (lvl < level_) {
			return;
		}
		CharBuffer buf;
		buf.append(LOG_LEVEL_STRINGS[static_cast<u8>(lvl)]);
		buf.append(msg);
		buf.append('\n');
		file_.write(StringView(buf.begin(), buf.end()));
	}

	static void flush();

	void trace3(const StringView& msg) { log(LogLevel::Trace3, msg); }
	void trace2(const StringView& msg) { log(LogLevel::Trace2, msg); }
	void trace1(const StringView& msg) { log(LogLevel::Trace1, msg); }
	void debug(const StringView& msg) { log(LogLevel::Debug, msg); }
	void info(const StringView& msg) { log(LogLevel::Info, msg); }
	void notice(const StringView& msg) { log(LogLevel::Notice, msg); }
	void warn(const StringView& msg) { log(LogLevel::Warning, msg); }
	void error(const StringView& msg) { log(LogLevel::Error, msg); }
	void critical(const StringView& msg) { log(LogLevel::Critical, msg); }

	template <typename... Args> void trace3(const FormatString<Args...>& fmt, Args&&... args) {
		log(LogLevel::Trace3, fmt, base::forward<Args>(args)...);
	}
	template <typename... Args> void trace2(const FormatString<Args...>& fmt, Args&&... args) {
		log(LogLevel::Trace2, fmt, base::forward<Args>(args)...);
	}
	template <typename... Args> void trace1(const FormatString<Args...>& fmt, Args&&... args) {
		log(LogLevel::Trace1, fmt, base::forward<Args>(args)...);
	}
	template <typename... Args> void debug(const FormatString<Args...>& fmt, Args&&... args) {
		log(LogLevel::Debug, fmt, base::forward<Args>(args)...);
	}
	template <typename... Args> void info(const FormatString<Args...>& fmt, Args&&... args) {
		log(LogLevel::Info, fmt, base::forward<Args>(args)...);
	}
	template <typename... Args> void notice(const FormatString<Args...>& fmt, Args&&... args) {
		log(LogLevel::Notice, fmt, base::forward<Args>(args)...);
	}
	template <typename... Args> void warn(const FormatString<Args...>& fmt, Args&&... args) {
		log(LogLevel::Warning, fmt, base::forward<Args>(args)...);
	}
	template <typename... Args> void error(const FormatString<Args...>& fmt, Args&&... args) {
		log(LogLevel::Error, fmt, base::forward<Args>(args)...);
	}
	template <typename... Args> void critical(const FormatString<Args...>& fmt, Args&&... args) {
		log(LogLevel::Critical, fmt, base::forward<Args>(args)...);
	}

private:
	File		 file_;
	LogLevel level_ = DEFAULT_LOG_LEVEL;
};

} // namespace base
