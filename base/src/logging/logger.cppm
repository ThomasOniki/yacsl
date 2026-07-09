module;

#include <sched.h>

export module base.logging.logger;

import base.logging.common;
import base.formatting.format_string;
import base.os.file;
import base.utilities.type_aliases;
import base.memory.lifetimes;
import base.ranges.string_view;
import base.containers.string;
import base.containers.option;
import base.threading.thread;
import base.threading.mpsc_ring_buffer;
import base.threading.atomic;

export namespace base {

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

  ~Logger() noexcept { work_.store(Work::Stop, MemoryOrder::Release); }

  Logger(const Logger&)            = delete;
  Logger(Logger&&)                 = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&)      = delete;

  [[nodiscard]] static Logger& get_default_instance() noexcept;

  [[nodiscard]] LogLevel level() const noexcept { return level_; }

  void flush() noexcept {
    work_.store(Work::Flush, MemoryOrder::Release);
    trace3("[Logger] flushing...");
    while (work_.load(MemoryOrder::Acquire) == Work::Flush) {
      sched_yield();
    }
  }

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

  void log(LogLevel lvl, const String& formatted) noexcept {
    if (lvl < level_) {
      return;
    }
    ring_.emplace(formatted, lvl);
  }

  template <typename... Args>
  void log(LogLevel lvl, const FormatString<Args...>& fmt, Args&&... args) {
    if (lvl < level_) {
      return;
    }
    String buf;
    format_to(buf, fmt, base::forward<Args>(args)...);
    log(lvl, buf);
  }

private:
  struct Message {
    alignas(32) String formatted;
    LogLevel level;
  };

  void write_message(const Message& msg) {
    if (msg.level >= LogLevel::Error) {
      String     buf;
      StringView lvl_str = err_lvl_strings_[static_cast<u8>(msg.level)];
      char*      data    = buf.grow(lvl_str.size() + msg.formatted.size() + 1);
      copy_bytes(data, lvl_str);
      copy_bytes(data + lvl_str.size(), msg.formatted.view());
      data[lvl_str.size() + msg.formatted.size()] = '\n';
      error_.write(buf.view());
    } else {
      String     buf;
      StringView lvl_str = out_lvl_strings_[static_cast<u8>(msg.level)];
      char*      data    = buf.grow(lvl_str.size() + msg.formatted.size() + 1);
      copy_bytes(data, lvl_str);
      copy_bytes(data + lvl_str.size(), msg.formatted.view());
      data[lvl_str.size() + msg.formatted.size()] = '\n';
      output_.write(buf.view());
    }
  }

  static void* work(void* pvoid) noexcept {
    Logger& self = *static_cast<Logger*>(pvoid);
    for (;;) {
      Work status = self.work_.load(MemoryOrder::Acquire);
      if (status != Work::Spin) {
        for (Option<Message> option = self.ring_.try_pop(); option.is_value();
             option                 = self.ring_.try_pop()) {
          self.write_message(option.assert_value());
        }

        if (status == Work::Stop) {
          break;
        }

        self.work_.store(Work::Spin, MemoryOrder::Release);
      }
      self.write_message(self.ring_.pop());
    }
    return nullptr;
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
  MPSCRingBuffer<Message, 64> ring_;
  File                        output_;
  File                        error_;
  Thread                      worker_{work, static_cast<void*>(this)};
  const StringView*           out_lvl_strings_;
  const StringView*           err_lvl_strings_;

  enum class Work : u8 {
    Spin  = 0,
    Flush = 1,
    Stop  = 2,
  };

  Atomic<Work> work_  = Work::Spin;
  LogLevel     level_ = DEFAULT_LOG_LEVEL;
  Flags        flags_;
};

// lvl is used to determine if it needs to output to the output file or error file.
void log(LogLevel lvl, const String& formatted) noexcept {
  Logger::get_default_instance().log(lvl, formatted);
}
template <typename... Args>
void log(LogLevel lvl, const FormatString<Args...>& fmt, Args&&... args) noexcept {
  Logger::get_default_instance().log(lvl, fmt, base::forward<Args>(args)...);
}

void log_trace3(const String& formatted) noexcept { log(LogLevel::Trace3, formatted); }
void log_trace2(const String& formatted) noexcept { log(LogLevel::Trace2, formatted); }
void log_trace1(const String& formatted) noexcept { log(LogLevel::Trace1, formatted); }
void log_debug(const String& formatted) noexcept { log(LogLevel::Debug, formatted); }
void log_info(const String& formatted) noexcept { log(LogLevel::Info, formatted); }
void log_notice(const String& formatted) noexcept { log(LogLevel::Notice, formatted); }
void log_warning(const String& formatted) noexcept { log(LogLevel::Warning, formatted); }
void log_error(const String& formatted) noexcept { log(LogLevel::Error, formatted); }

template <typename... Args>
void log_trace3(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(LogLevel::Trace3, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_trace2(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(LogLevel::Trace2, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_trace1(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(LogLevel::Trace1, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_debug(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(LogLevel::Debug, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_info(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(LogLevel::Info, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_notice(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(LogLevel::Notice, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_warning(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(LogLevel::Warning, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_error(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(LogLevel::Error, fmt, base::forward<Args>(args)...);
}

} // namespace base
