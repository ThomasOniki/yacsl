#pragma once

#include "base/containers/char_buffer.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/common.hpp"
#include "base/logging/format_string.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class Logger {
public:
  Logger() noexcept  = default;
  ~Logger() noexcept = default;
  explicit Logger(LogLevel level) noexcept : level_(level) {}

  Logger(const Logger&)            = delete;
  Logger(Logger&&)                 = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&)      = delete;

  static Logger& get_default_instance();

  static void flush();

  void trace3(const StringView& msg) { log(LogLevel::Trace3, msg); }
  void trace2(const StringView& msg) { log(LogLevel::Trace2, msg); }
  void trace1(const StringView& msg) { log(LogLevel::Trace1, msg); }
  void debug(const StringView& msg) { log(LogLevel::Debug, msg); }
  void info(const StringView& msg) { log(LogLevel::Info, msg); }
  void notice(const StringView& msg) { log(LogLevel::Notice, msg); }
  void warn(const StringView& msg) { log(LogLevel::Warning, msg); }
  void error(const StringView& msg) { log(LogLevel::Error, msg); }
  void panic(const StringView& msg) { log(LogLevel::Panic, msg); }

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
  template <typename... Args> void panic(const FormatString<Args...>& fmt, Args&&... args) {
    log(LogLevel::Panic, fmt, base::forward<Args>(args)...);
  }

  template <typename... Args>
  void log(LogLevel lvl, const FormatString<Args...>& fmt, Args&&... args) {
    if (lvl < level_) {
      return;
    }
    CharBuffer buf;
    buf.append(COLORED_LOG_LEVEL_STRINGS[static_cast<u8>(lvl)]);
    format_to(buf, fmt, base::forward<Args>(args)...);
    buf.append('\n');
    if (lvl >= LogLevel::Error) {
      write_to_stderr(StringView(buf.begin(), buf.end()));
    } else {
      write_to_stdout(StringView(buf.begin(), buf.end()));
    }
  }

  void log(LogLevel lvl, const StringView& msg) {
    if (lvl < level_) {
      return;
    }
    CharBuffer buf;
    buf.append(COLORED_LOG_LEVEL_STRINGS[static_cast<u8>(lvl)]);
    buf.append(msg);
    buf.append('\n');
    if (lvl >= LogLevel::Error) {
      write_to_stderr(StringView(buf.begin(), buf.end()));
    } else {
      write_to_stdout(StringView(buf.begin(), buf.end()));
    }
  }

private:
  static void write_to_stdout(const StringView& str);
  static void write_to_stderr(const StringView& str);

private:
  LogLevel level_ = DEFAULT_LOG_LEVEL;
};

template <typename... Args> void log_trace3(const FormatString<Args...>& fmt, Args&&... args) {
  Logger::get_default_instance().trace3(fmt, base::forward<Args>(args)...);
}
template <typename... Args> void log_trace2(const FormatString<Args...>& fmt, Args&&... args) {
  Logger::get_default_instance().trace2(fmt, base::forward<Args>(args)...);
}
template <typename... Args> void log_trace1(const FormatString<Args...>& fmt, Args&&... args) {
  Logger::get_default_instance().trace1(fmt, base::forward<Args>(args)...);
}
template <typename... Args> void log_debug(const FormatString<Args...>& fmt, Args&&... args) {
  Logger::get_default_instance().debug(fmt, base::forward<Args>(args)...);
}
template <typename... Args> void log_info(const FormatString<Args...>& fmt, Args&&... args) {
  Logger::get_default_instance().info(fmt, base::forward<Args>(args)...);
}
template <typename... Args> void log_notice(const FormatString<Args...>& fmt, Args&&... args) {
  Logger::get_default_instance().notice(fmt, base::forward<Args>(args)...);
}
template <typename... Args> void log_warning(const FormatString<Args...>& fmt, Args&&... args) {
  Logger::get_default_instance().warn(fmt, base::forward<Args>(args)...);
}
template <typename... Args> void log_error(const FormatString<Args...>& fmt, Args&&... args) {
  Logger::get_default_instance().error(fmt, base::forward<Args>(args)...);
}

} // namespace base
