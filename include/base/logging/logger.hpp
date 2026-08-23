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
    alignas(32) Slice<StringView const, u32> output;
    Slice<StringView const, u32> error;
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

  Logger(Logger const&)            = delete;
  Logger(Logger&&)                 = delete;
  Logger& operator=(Logger const&) = delete;
  Logger& operator=(Logger&&)      = delete;

  [[nodiscard]] static Logger& get_default_instance() noexcept;

  [[nodiscard]] Level level(this Logger const& self) noexcept { return self.level_; }

  void flush(this Logger& self) noexcept {
    self.work_.store(Work::Flush, MemoryOrder::Release);
    self.trace3("[Logger] flushing...");
    while (self.work_.load(MemoryOrder::Acquire) == Work::Flush) {
      sched_yield();
    }
  }

  template <typename... Args>
  void trace3(this Logger& self, FormatString<Args...> const& fmt, Args&&... args) {
    self.log(Level::Trace3, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args>
  void trace2(this Logger& self, FormatString<Args...> const& fmt, Args&&... args) {
    self.log(Level::Trace2, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args>
  void trace1(this Logger& self, FormatString<Args...> const& fmt, Args&&... args) {
    self.log(Level::Trace1, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args>
  void debug(this Logger& self, FormatString<Args...> const& fmt, Args&&... args) {
    self.log(Level::Debug, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args>
  void info(this Logger& self, FormatString<Args...> const& fmt, Args&&... args) {
    self.log(Level::Info, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args>
  void notice(this Logger& self, FormatString<Args...> const& fmt, Args&&... args) {
    self.log(Level::Notice, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args>
  void warn(this Logger& self, FormatString<Args...> const& fmt, Args&&... args) {
    self.log(Level::Warning, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args>
  void error(this Logger& self, FormatString<Args...> const& fmt, Args&&... args) {
    self.log(Level::Error, fmt, base::forward<Args>(args)...);
  }
  template <typename... Args>
  void panic(this Logger& self, FormatString<Args...> const& fmt, Args&&... args) {
    self.log(Level::Panic, fmt, base::forward<Args>(args)...);
  }

  void log(this Logger& self, Level lvl, String const& formatted) noexcept {
    if (lvl < self.level_) {
      return;
    }
    self.ring_.emplace(formatted, lvl);
  }

  template <typename... Args>
  void log(this Logger& self, Level lvl, FormatString<Args...> const& fmt, Args&&... args) {
    if (lvl < self.level_) {
      return;
    }
    String buf;
    format_to(buf, fmt, base::forward<Args>(args)...);
    self.log(lvl, buf);
  }

private:
  struct Message {
    alignas(32) String formatted;
    Level level;
  };

  void write_message(this Logger& self, Message const& msg) {
    if (msg.level >= Level::Error) {
      String           buf;
      StringView const lvl_str = self.log_lvl_strings_.error[static_cast<u8>(msg.level)];
      char*            data    = buf.grow(lvl_str.size() + msg.formatted.size() + 1);
      copy(lvl_str.slice(), data);
      copy(msg.formatted.slice(), data + lvl_str.size());
      data[lvl_str.size() + msg.formatted.size()] = '\n';
      self.error_.write(buf.bytes());
    } else {
      String           buf;
      StringView const lvl_str = self.log_lvl_strings_.output[static_cast<u8>(msg.level)];
      char*            data    = buf.grow(lvl_str.size() + msg.formatted.size() + 1);
      copy(lvl_str.slice(), data);
      copy(msg.formatted.slice(), data + lvl_str.size());
      data[lvl_str.size() + msg.formatted.size()] = '\n';
      self.output_.write(buf.bytes());
    }
  }

  static void* work(void* pvoid) noexcept {
    Logger& self = *static_cast<Logger*>(pvoid);
    for (;;) {
      Work const work = self.work_.load(MemoryOrder::Acquire);
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
inline void log(Logger::Level lvl, String const& formatted) noexcept {
  Logger::get_default_instance().log(lvl, formatted);
}
template <typename... Args>
void log(Logger::Level lvl, FormatString<Args...> const& fmt, Args&&... args) noexcept {
  Logger::get_default_instance().log(lvl, fmt, base::forward<Args>(args)...);
}

inline void log_trace3(String const& formatted) noexcept { log(Logger::Level::Trace3, formatted); }
inline void log_trace2(String const& formatted) noexcept { log(Logger::Level::Trace2, formatted); }
inline void log_trace1(String const& formatted) noexcept { log(Logger::Level::Trace1, formatted); }
inline void log_debug(String const& formatted) noexcept { log(Logger::Level::Debug, formatted); }
inline void log_info(String const& formatted) noexcept { log(Logger::Level::Info, formatted); }
inline void log_notice(String const& formatted) noexcept { log(Logger::Level::Notice, formatted); }
inline void log_warning(String const& formatted) noexcept {
  log(Logger::Level::Warning, formatted);
}
inline void log_error(String const& formatted) noexcept { log(Logger::Level::Error, formatted); }

template <typename... Args>
void log_trace3(FormatString<Args...> const& fmt, Args&&... args) noexcept {
  log(Logger::Level::Trace3, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_trace2(FormatString<Args...> const& fmt, Args&&... args) noexcept {
  log(Logger::Level::Trace2, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_trace1(FormatString<Args...> const& fmt, Args&&... args) noexcept {
  log(Logger::Level::Trace1, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_debug(FormatString<Args...> const& fmt, Args&&... args) noexcept {
  log(Logger::Level::Debug, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_info(FormatString<Args...> const& fmt, Args&&... args) noexcept {
  log(Logger::Level::Info, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_notice(FormatString<Args...> const& fmt, Args&&... args) noexcept {
  log(Logger::Level::Notice, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_warning(FormatString<Args...> const& fmt, Args&&... args) noexcept {
  log(Logger::Level::Warning, fmt, base::forward<Args>(args)...);
}
template <typename... Args>
void log_error(FormatString<Args...> const& fmt, Args&&... args) noexcept {
  log(Logger::Level::Error, fmt, base::forward<Args>(args)...);
}

} // namespace base
