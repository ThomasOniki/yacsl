#pragma once

#include "base/containers/char_buffer.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/common.hpp"
#include "base/logging/format_string.hpp"
#include "base/sys/file.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class Logger {
public:
  enum class Flags : u8 {
    ColorOutput = 1 << 0,
    ColorError  = 1 << 1,
  };

  friend constexpr Flags operator|(Flags rhs, Flags lhs) noexcept {
    return static_cast<Flags>(static_cast<u8>(rhs) | static_cast<u8>(lhs));
  }

  Logger(File&& output, File&& error, LogLevel lvl, Flags flags) noexcept
      : output_(base::move(output)), error_(base::move(error)),
        out_lvl_strings_(out_lvl_strings(flags)), err_lvl_strings_(err_lvl_strings(flags)),
        level_(lvl), flags_(flags) {}

  Logger(LogLevel lvl, Flags flags) noexcept
      : out_lvl_strings_(out_lvl_strings(flags)), err_lvl_strings_(err_lvl_strings(flags)),
        level_(lvl), flags_(flags) {
    output_.open_stdout();
    error_.open_stderr();
  }

  Logger(File&& output, File&& error, LogLevel lvl) noexcept
      : Logger(base::move(output), base::move(error), lvl, Flags::ColorOutput | Flags::ColorError) {
  }
  Logger(File&& output, File&& error, Flags flags) noexcept
      : Logger(base::move(output), base::move(error), DEFAULT_LOG_LEVEL, flags) {}
  Logger(File&& output, File&& error) noexcept
      : Logger(base::move(output), base::move(error), DEFAULT_LOG_LEVEL) {}

  explicit Logger(LogLevel lvl) noexcept : Logger(lvl, Flags::ColorOutput | Flags::ColorError) {}
  explicit Logger(Flags flags) noexcept : Logger(DEFAULT_LOG_LEVEL, flags) {}

  Logger() : Logger(DEFAULT_LOG_LEVEL, Flags::ColorOutput | Flags::ColorError) {}

  ~Logger() noexcept { flush(); }

  Logger(const Logger&)            = delete;
  Logger(Logger&&)                 = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&)      = delete;

  static Logger& get_default_instance() noexcept;

  void flush() noexcept {
    if (out_buf_.size() > 0) {
      output_.write(StringView(out_buf_.data(), out_buf_.size()));
      out_buf_.clear();
    }
  }

  void trace3(StringView msg) { log(LogLevel::Trace3, msg); }
  void trace2(StringView msg) { log(LogLevel::Trace2, msg); }
  void trace1(StringView msg) { log(LogLevel::Trace1, msg); }
  void debug(StringView msg) { log(LogLevel::Debug, msg); }
  void info(StringView msg) { log(LogLevel::Info, msg); }
  void notice(StringView msg) { log(LogLevel::Notice, msg); }
  void warn(StringView msg) { log(LogLevel::Warning, msg); }
  void error(StringView msg) { log(LogLevel::Error, msg); }
  void panic(StringView msg) { log(LogLevel::Panic, msg); }

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
    if (lvl < LogLevel::Error) {
      out_buf_.append(out_lvl_strings_[static_cast<u8>(lvl)]);
      format_to(out_buf_, fmt, base::forward<Args>(args)...);
      out_buf_.append('\n');
      maybe_flush();
    } else {
      CharBuffer buf;
      buf.append(err_lvl_strings_[static_cast<u8>(lvl)]);
      format_to(buf, fmt, base::forward<Args>(args)...);
      buf.append('\n');
      error_.write(StringView(buf.begin(), buf.end()));
    }
  }

  void log(LogLevel lvl, StringView msg) {
    if (lvl < level_) {
      return;
    }
    if (lvl < LogLevel::Error) {
      out_buf_.append(out_lvl_strings_[static_cast<u8>(lvl)]);
      out_buf_.append(msg);
      out_buf_.append('\n');
      maybe_flush();
    } else {
      CharBuffer buf;
      buf.append(err_lvl_strings_[static_cast<u8>(lvl)]);
      buf.append(msg);
      buf.append('\n');
      error_.write(StringView(buf.begin(), buf.end()));
    }
  }

private:
  static void write_to_stderr(StringView str);

  void maybe_flush() noexcept {
    if (out_buf_.size() > 4096) {
      output_.write(StringView(out_buf_.data(), out_buf_.size()));
    }
  }

  static const StringView* out_lvl_strings(Flags flags) {
    return (static_cast<u8>(flags) & static_cast<u8>(Flags::ColorOutput)) != 0
               ? COLORED_LOG_LEVEL_STRINGS.data()
               : LOG_LEVEL_STRINGS.data();
  }
  static const StringView* err_lvl_strings(Flags flags) {
    return (static_cast<u8>(flags) & static_cast<u8>(Flags::ColorError)) != 0
               ? COLORED_LOG_LEVEL_STRINGS.data()
               : LOG_LEVEL_STRINGS.data();
  }

private:
  CharBuffer        out_buf_;
  File              output_;
  File              error_;
  const StringView* out_lvl_strings_;
  const StringView* err_lvl_strings_;
  LogLevel          level_ = DEFAULT_LOG_LEVEL;
  Flags             flags_;
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
