#pragma once

#include "yacsl/containers/option.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/export.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/os/file.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/threads/atomic.hpp"
#include "yacsl/threads/mpsc_ring_buffer.hpp"
#include "yacsl/threads/thread.hpp"
#include "yacsl/type_aliases.hpp"
#include <sched.h>

namespace yacsl::debug {

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

  static constexpr containers::StackArray<ranges::StringView, 10>
      LEVEL_STRINGS = {
          "[TRACE 3] ", "[TRACE 2] ", "[TRACE 1] ", "[DEBUG] ", "[INFO] ",
          "[NOTICE] ",  "[WARNING] ", "[ERROR] ",   "[PANIC] ", "",
  };

  static constexpr containers::StackArray<ranges::StringView, 10>
      COLORED_LEVEL_STRINGS = {
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
    alignas(32) ranges::Slice<ranges::StringView const, u32> output;
    ranges::Slice<ranges::StringView const, u32> error;
  };

  Logger(os::File&& output, os::File&& error, Level lvl,
         LevelStrings log_level_strings) noexcept
      : output_(memory::move(output)), error_(memory::move(error)),
        log_lvl_strings_(log_level_strings), level_(lvl) {}
  Logger(os::File&& output, os::File&& error, Level lvl) noexcept
      : output_(memory::move(output)), error_(memory::move(error)),
        log_lvl_strings_{
            .output = output_.is_tty() ? COLORED_LEVEL_STRINGS.slice()
                                       : LEVEL_STRINGS.slice(),
            .error  = error_.is_tty() ? COLORED_LEVEL_STRINGS.slice()
                                      : LEVEL_STRINGS.slice(),
        },
        level_(lvl) {}
  Logger(os::File&& output, os::File&& error,
         LevelStrings log_level_strings) noexcept
      : Logger(memory::move(output), memory::move(error), DEFAULT_LOG_LEVEL,
               log_level_strings) {}
  Logger(os::File&& output, os::File&& error) noexcept
      : Logger(memory::move(output), memory::move(error), DEFAULT_LOG_LEVEL) {}

  Logger(Level lvl, LevelStrings log_level_strings) noexcept
      : output_(os::File::open_stdout()), error_(os::File::open_stderr()),
        log_lvl_strings_(log_level_strings), level_(lvl) {}

  explicit Logger(Level lvl) noexcept
      : output_(os::File::open_stdout()), error_(os::File::open_stderr()),
        log_lvl_strings_({
            .output = output_.is_tty() ? COLORED_LEVEL_STRINGS.slice()
                                       : LEVEL_STRINGS.slice(),
            .error  = error_.is_tty() ? COLORED_LEVEL_STRINGS.slice()
                                      : LEVEL_STRINGS.slice(),
        }),
        level_(lvl) {}
  Logger() : Logger(DEFAULT_LOG_LEVEL) {}

  ~Logger() noexcept { work_.store(Work::Stop, threads::MemoryOrder::Release); }

  Logger(Logger const&)            = delete;
  Logger(Logger&&)                 = delete;
  Logger& operator=(Logger const&) = delete;
  Logger& operator=(Logger&&)      = delete;

  [[nodiscard]] static Logger& get_default_instance() noexcept YACSL_API;

  [[nodiscard]] Level level(this Logger const& self) noexcept {
    return self.level_;
  }

  void flush(this Logger& self) noexcept {
    self.work_.store(Work::Flush, threads::MemoryOrder::Release);
    self.ring_.push({.formatted = "", .level = Level::None});
    while (self.work_.load(threads::MemoryOrder::Acquire) == Work::Flush) {
      sched_yield();
    }
  }

  template <typename... Args>
  void trace3(this Logger& self, fmt::FormatString<Args...> const& fmt,
              Args&&... args) {
    self.log(Level::Trace3, fmt, memory::forward<Args>(args)...);
  }
  template <typename... Args>
  void trace2(this Logger& self, fmt::FormatString<Args...> const& fmt,
              Args&&... args) {
    self.log(Level::Trace2, fmt, memory::forward<Args>(args)...);
  }
  template <typename... Args>
  void trace1(this Logger& self, fmt::FormatString<Args...> const& fmt,
              Args&&... args) {
    self.log(Level::Trace1, fmt, memory::forward<Args>(args)...);
  }
  template <typename... Args>
  void debug(this Logger& self, fmt::FormatString<Args...> const& fmt,
             Args&&... args) {
    self.log(Level::Debug, fmt, memory::forward<Args>(args)...);
  }
  template <typename... Args>
  void info(this Logger& self, fmt::FormatString<Args...> const& fmt,
            Args&&... args) {
    self.log(Level::Info, fmt, memory::forward<Args>(args)...);
  }
  template <typename... Args>
  void notice(this Logger& self, fmt::FormatString<Args...> const& fmt,
              Args&&... args) {
    self.log(Level::Notice, fmt, memory::forward<Args>(args)...);
  }
  template <typename... Args>
  void warn(this Logger& self, fmt::FormatString<Args...> const& fmt,
            Args&&... args) {
    self.log(Level::Warning, fmt, memory::forward<Args>(args)...);
  }
  template <typename... Args>
  void error(this Logger& self, fmt::FormatString<Args...> const& fmt,
             Args&&... args) {
    self.log(Level::Error, fmt, memory::forward<Args>(args)...);
  }
  template <typename... Args>
  void panic(this Logger& self, fmt::FormatString<Args...> const& fmt,
             Args&&... args) {
    self.log(Level::Panic, fmt, memory::forward<Args>(args)...);
  }

  void log(this Logger& self, Level lvl,
           containers::String const& formatted) noexcept {
    if (lvl < self.level_) {
      return;
    }
    self.ring_.emplace(formatted, lvl);
  }

  template <typename... Args>
  void log(this Logger& self, Level lvl, fmt::FormatString<Args...> const& fmt,
           Args&&... args) {
    if (lvl < self.level_) {
      return;
    }
    self.log(lvl, fmt::format(fmt, memory::forward<Args>(args)...));
  }

private:
  struct Message {
    alignas(32) containers::String formatted;
    Level level;
  };

  void write_message(this Logger& self, Message const& msg) {
    if (msg.level >= Level::Error) {
      if (msg.level == Level::None) {
        self.error_.write(msg.formatted.bytes());
      } else {
        containers::String       buf;
        ranges::StringView const lvl_str =
            self.log_lvl_strings_.error[static_cast<u8>(msg.level)];
        char* data = buf.grow(lvl_str.size() + msg.formatted.size() + 1);
        memory::copy(lvl_str.slice(), data);
        memory::copy(msg.formatted.slice(), data + lvl_str.size());
        data[lvl_str.size() + msg.formatted.size()] = '\n';
        self.error_.write(buf.bytes());
      }
    } else {
      containers::String       buf;
      ranges::StringView const lvl_str =
          self.log_lvl_strings_.output[static_cast<u8>(msg.level)];
      char* data = buf.grow(lvl_str.size() + msg.formatted.size() + 1);
      memory::copy(lvl_str.slice(), data);
      memory::copy(msg.formatted.slice(), data + lvl_str.size());
      data[lvl_str.size() + msg.formatted.size()] = '\n';
      self.output_.write(buf.bytes());
    }
  }

  static void* work(void* pvoid) noexcept {
    Logger& self = *static_cast<Logger*>(pvoid);
    for (;;) {
      Work const work = self.work_.load(threads::MemoryOrder::Acquire);
      if (work == Work::Spin) {
        self.write_message(self.ring_.pop());
      } else {
        for (containers::Option<Message> option = self.ring_.try_pop();
             option.is_value(); option          = self.ring_.try_pop()) {
          self.write_message(option.value_unchecked());
        }

        if (work == Work::Stop) {
          break;
        }

        self.work_.store(Work::Spin, threads::MemoryOrder::Release);
      }
    }
    return nullptr;
  }

private:
  threads::MPSCRingBuffer<Message, 64> ring_;
  os::File                             output_;
  os::File                             error_;
  LevelStrings                         log_lvl_strings_;

  enum class Work : u8 {
    Spin  = 0,
    Flush = 1,
    Stop  = 2,
  };
  threads::Thread       worker_{work, static_cast<void*>(this)};
  threads::Atomic<Work> work_  = Work::Spin;
  Level                 level_ = DEFAULT_LOG_LEVEL;
};

// lvl is used to determine if it needs to output to the output file or error
// file.
inline void log(Logger::Level             lvl,
                containers::String const& formatted) noexcept {
  Logger::get_default_instance().log(lvl, formatted);
}
template <typename... Args>
void log(Logger::Level lvl, fmt::FormatString<Args...> const& fmt,
         Args&&... args) noexcept {
  Logger::get_default_instance().log(lvl, fmt, memory::forward<Args>(args)...);
}

inline void log_trace3(containers::String const& formatted) noexcept {
  log(Logger::Level::Trace3, formatted);
}
inline void log_trace2(containers::String const& formatted) noexcept {
  log(Logger::Level::Trace2, formatted);
}
inline void log_trace1(containers::String const& formatted) noexcept {
  log(Logger::Level::Trace1, formatted);
}
inline void log_debug(containers::String const& formatted) noexcept {
  log(Logger::Level::Debug, formatted);
}
inline void log_info(containers::String const& formatted) noexcept {
  log(Logger::Level::Info, formatted);
}
inline void log_notice(containers::String const& formatted) noexcept {
  log(Logger::Level::Notice, formatted);
}
inline void log_warning(containers::String const& formatted) noexcept {
  log(Logger::Level::Warning, formatted);
}
inline void log_error(containers::String const& formatted) noexcept {
  log(Logger::Level::Error, formatted);
}

template <typename... Args>
void log_trace3(fmt::FormatString<Args...> const& fmt,
                Args&&... args) noexcept {
  log(Logger::Level::Trace3, fmt, memory::forward<Args>(args)...);
}
template <typename... Args>
void log_trace2(fmt::FormatString<Args...> const& fmt,
                Args&&... args) noexcept {
  log(Logger::Level::Trace2, fmt, memory::forward<Args>(args)...);
}
template <typename... Args>
void log_trace1(fmt::FormatString<Args...> const& fmt,
                Args&&... args) noexcept {
  log(Logger::Level::Trace1, fmt, memory::forward<Args>(args)...);
}
template <typename... Args>
void log_debug(fmt::FormatString<Args...> const& fmt, Args&&... args) noexcept {
  log(Logger::Level::Debug, fmt, memory::forward<Args>(args)...);
}
template <typename... Args>
void log_info(fmt::FormatString<Args...> const& fmt, Args&&... args) noexcept {
  log(Logger::Level::Info, fmt, memory::forward<Args>(args)...);
}
template <typename... Args>
void log_notice(fmt::FormatString<Args...> const& fmt,
                Args&&... args) noexcept {
  log(Logger::Level::Notice, fmt, memory::forward<Args>(args)...);
}
template <typename... Args>
void log_warning(fmt::FormatString<Args...> const& fmt,
                 Args&&... args) noexcept {
  log(Logger::Level::Warning, fmt, memory::forward<Args>(args)...);
}
template <typename... Args>
void log_error(fmt::FormatString<Args...> const& fmt, Args&&... args) noexcept {
  log(Logger::Level::Error, fmt, memory::forward<Args>(args)...);
}

} // namespace yacsl::debug
