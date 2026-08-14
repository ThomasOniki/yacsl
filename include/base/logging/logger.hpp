#pragma once

#include "base/containers/option.hpp"
#include "base/containers/string.hpp"
#include "base/formatting/format_string.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/os/file.hpp"
#include "base/ranges/string_view.hpp"
#include "base/threading/atomic.hpp"
#include "base/threading/mpsc_ring_buffer.hpp"
#include "base/threading/thread.hpp"
#include "base/utilities/type_aliases.hpp"
#include <sched.h>

namespace base {

class Logger {
public:
  enum class Level : u8 {
    Trace3  = 0,
    Trace2  = 1,
    Trace1  = 2,
    Debug   = 3,
    Info    = 4,
    Notice  = 5,
    Warning = 6,
    Error   = 7,
    Panic   = 8,
    None    = 9,
  };

  static constexpr StackArray<StringView, 10> LEVEL_STRINGS = {
      "[TRACE 3] ", "[TRACE 2] ", "[TRACE 1] ", "[DEBUG] ", "[INFO] ",
      "[NOTICE] ",  "[WARNING] ", "[ERROR] ",   "[PANIC] ", "",
  };

  static constexpr StackArray<StringView, 10> COLORED_LEVEL_STRINGS = {
      "[TRACE 3] ",          "[TRACE 2] ",
      "[TRACE 1] ",          "[[36mDEBUG[0m] ",
      "[[32mINFO[0m] ",    "[[34mNOTICE[0m] ",
      "[[33mWARNING[0m] ", "[[31mERROR[0m] ",
      "[[41mPANIC[0m] ",   "",
  };

#ifndef NDEBUG
  static constexpr Level DEFAULT_LOG_LEVEL = Level::Trace3;
#else
  static constexpr Level DEFAULT_LOG_LEVEL = Level::Info;
#endif
  struct LevelStrings {
    alignas(32) Slice<const StringView, u32> output;
    Slice<const StringView, u32> error;
  };

  Logger(File&& output, File&& error, Level lvl, LevelStrings log_level_strings) noexcept
      : output_(base::move(output)), error_(base::move(error)), log_lvl_strings_(log_level_strings),
        level_(lvl) {}
  Logger(File&& output, File&& error, Level lvl) noexcept
      : output_(base::move(output)), error_(base::move(error)),
        log_lvl_strings_{
            .output = output_.is_tty() ? COLORED_LEVEL_STRINGS.slice() : LEVEL_STRINGS.slice(),
            .error  = error_.is_tty() ? COLORED_LEVEL_STRINGS.slice() : LEVEL_STRINGS.slice()},
        level_(lvl) {}
  Logger(File&& output, File&& error, LevelStrings log_level_strings) noexcept
      : Logger(base::move(output), base::move(error), DEFAULT_LOG_LEVEL, log_level_strings) {}
  Logger(File&& output, File&& error) noexcept
      : Logger(base::move(output), base::move(error), DEFAULT_LOG_LEVEL) {}

  Logger(Level lvl, LevelStrings log_level_strings) noexcept
      : log_lvl_strings_{log_level_strings}, level_(lvl) {
    output_.open_stdout();
    error_.open_stderr();
  }

  explicit Logger(Level lvl) noexcept
      : log_lvl_strings_{.output = LEVEL_STRINGS.slice(), .error = LEVEL_STRINGS.slice()},
        level_(lvl) {
    output_.open_stdout();
    error_.open_stderr();
    log_lvl_strings_ = {
        .output = output_.is_tty() ? COLORED_LEVEL_STRINGS.slice() : LEVEL_STRINGS.slice(),
        .error  = error_.is_tty() ? COLORED_LEVEL_STRINGS.slice() : LEVEL_STRINGS.slice(),
    };
  }
  Logger() : Logger(DEFAULT_LOG_LEVEL) {}

  ~Logger() noexcept { work_.store(Work::Stop, MemoryOrder::Release); }

  Logger(const Logger&)            = delete;
  Logger(Logger&&)                 = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&)      = delete;

  [[nodiscard]] static Logger& get_default_instance() noexcept;

  [[nodiscard]] Level level() const noexcept { return level_; }

  void flush() noexcept {
    work_.store(Work::Flush, MemoryOrder::Release);
    trace3("[Logger] flushing...");
    while (work_.load(MemoryOrder::Acquire) == Work::Flush) {
      sched_yield();
    }
  }

  template <typename... Args> void trace3(const FormatString<Args...>& fmt, Args&&... args) {
    log(Level::Trace3, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args> void trace2(const FormatString<Args...>& fmt, Args&&... args) {
    log(Level::Trace2, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args> void trace1(const FormatString<Args...>& fmt, Args&&... args) {
    log(Level::Trace1, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args> void debug(const FormatString<Args...>& fmt, Args&&... args) {
    log(Level::Debug, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args> void info(const FormatString<Args...>& fmt, Args&&... args) {
    log(Level::Info, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args> void notice(const FormatString<Args...>& fmt, Args&&... args) {
    log(Level::Notice, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args> void warn(const FormatString<Args...>& fmt, Args&&... args) {
    log(Level::Warning, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args> void error(const FormatString<Args...>& fmt, Args&&... args) {
    log(Level::Error, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args> void panic(const FormatString<Args...>& fmt, Args&&... args) {
    log(Level::Panic, fmt, base::forward<Args>(args)...);
  }

  void log(Level lvl, const String& formatted) noexcept {
    if (lvl < level_) {
      return;
    }
    ring_.emplace(formatted, lvl);
  }

  template <typename... Args>
  void log(Level lvl, const FormatString<Args...>& fmt, Args&&... args) {
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
    Level level;
  };

  void write_message(const Message& msg) {
    if (msg.level >= Level::Error) {
      String           buf;
      const StringView lvl_str = log_lvl_strings_.error[static_cast<u8>(msg.level)];
      char*            data    = buf.grow(lvl_str.size() + msg.formatted.size() + 1);
      copy(lvl_str.slice(), data);
      copy(msg.formatted.slice(), data + lvl_str.size());
      data[lvl_str.size() + msg.formatted.size()] = '\n';
      error_.write(buf.bytes());
    } else {
      String           buf;
      const StringView lvl_str = log_lvl_strings_.output[static_cast<u8>(msg.level)];
      char*            data    = buf.grow(lvl_str.size() + msg.formatted.size() + 1);
      copy(lvl_str.slice(), data);
      copy(msg.formatted.slice(), data + lvl_str.size());
      data[lvl_str.size() + msg.formatted.size()] = '\n';
      output_.write(buf.bytes());
    }
  }

  static void* work(void* pvoid) noexcept {
    Logger& self = *static_cast<Logger*>(pvoid);
    for (;;) {
      const Work work = self.work_.load(MemoryOrder::Acquire);
      if (work != Work::Spin) {
        for (Option<Message> option = self.ring_.try_pop(); option.is_value();
             option                 = self.ring_.try_pop()) {
          self.write_message(option.assert_value());
        }

        if (work == Work::Stop) {
          break;
        }

        self.work_.store(Work::Spin, MemoryOrder::Release);
      } else {
        self.write_message(self.ring_.pop());
      }
    }
    return nullptr;
  }

private:
  MPSCRingBuffer<Message, 64> ring_;
  File                        output_;
  File                        error_;
  LevelStrings                log_lvl_strings_;

  enum class Work : u8 {
    Spin  = 0,
    Flush = 1,
    Stop  = 2,
  };
  Thread       worker_{work, static_cast<void*>(this)};
  Atomic<Work> work_  = Work::Spin;
  Level        level_ = DEFAULT_LOG_LEVEL;
};

// lvl is used to determine if it needs to output to the output file or error file.
inline void log(Logger::Level lvl, const String& formatted) noexcept {
  Logger::get_default_instance().log(lvl, formatted);
}
template <typename... Args>
void log(Logger::Level lvl, const FormatString<Args...>& fmt, Args&&... args) noexcept {
  Logger::get_default_instance().log(lvl, fmt, base::forward<Args>(args)...);
}

inline void log_trace3(const String& formatted) noexcept { log(Logger::Level::Trace3, formatted); }
inline void log_trace2(const String& formatted) noexcept { log(Logger::Level::Trace2, formatted); }
inline void log_trace1(const String& formatted) noexcept { log(Logger::Level::Trace1, formatted); }
inline void log_debug(const String& formatted) noexcept { log(Logger::Level::Debug, formatted); }
inline void log_info(const String& formatted) noexcept { log(Logger::Level::Info, formatted); }
inline void log_notice(const String& formatted) noexcept { log(Logger::Level::Notice, formatted); }
inline void log_warning(const String& formatted) noexcept { log(Logger::Level::Warning, formatted); }
inline void log_error(const String& formatted) noexcept { log(Logger::Level::Error, formatted); }

template <typename... Args>
void log_trace3(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(Logger::Level::Trace3, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_trace2(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(Logger::Level::Trace2, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_trace1(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(Logger::Level::Trace1, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_debug(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(Logger::Level::Debug, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_info(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(Logger::Level::Info, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_notice(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(Logger::Level::Notice, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_warning(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(Logger::Level::Warning, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_error(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  log(Logger::Level::Error, fmt, base::forward<Args>(args)...);
}

} // namespace base
