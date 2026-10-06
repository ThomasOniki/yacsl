#pragma once

#include "yacsl/containers/string.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::testing {

class Test {
public:
  // name is char const* so that Test doesn't need String to work, and can be
  // used to test String.
  constexpr Test(char const* name, void (*callback)(Test&))
      : name_(name), callback_(callback) {}

  enum class Result : u8 {
    Success,
    Failure,
  };

  Result run_quietly(this Test& self) noexcept {
    self.result_ = Result::Success;
    self.register_current_thread();
    self.callback_(self);
    return self.result_;
  }
  Result run(this Test& self) noexcept {
    debug::log_info("Running test {?}...", self.name_);
    if (self.run_quietly() == Result::Success) {
      debug::log_info("Test {?} suceeded!", self.name_);
    } else {
      debug::log_error("Test {?} failed!", self.name_);
    }
    debug::flush_logger();
    return self.result_;
  }

  [[nodiscard]] constexpr ranges::StringView
  name(this Test const& self) noexcept {
    return self.name_;
  }
  [[nodiscard]] constexpr char const*
  cstr_name(this Test const& self) noexcept {
    return self.name_;
  }

  [[nodiscard]] static constexpr Test& current_test() noexcept {
    return *CURRENT_TEST;
  }
  void register_current_thread(this Test& self) noexcept {
    CURRENT_TEST = &self;
  }

  constexpr void fail_quietly(this Test& self) noexcept {
    self.result_ = Result::Failure;
  }
  constexpr void fail(this Test&            self,
                      debug::SourceLocation location =
                          debug::SourceLocation::current()) noexcept {
    debug::log_error(fmt::format("Test {?} failed at {}\nstacktrace:\n{}",
                                 self.name_, location,
                                 debug::StackTrace::current()));
    self.result_ = Result::Failure;
  }
  constexpr void fail(this Test&                self,
                      containers::String const& formatted) noexcept {
    debug::log_error(formatted);
    self.result_ = Result::Failure;
  }
  template <typename... Args>
  constexpr void fail(this Test& self, fmt::FormatString<Args...> const& fmt,
                      Args&&... args) noexcept {
    debug::log_error(
        fmt::format("Test {?} failed at {}\n", CURRENT_TEST->name_,
                    fmt.source_location()) +
        fmt::format(fmt, memory::forward<Args>(args)...) +
        fmt::format("\nstacktrace:\n{}", debug::StackTrace::current()));
    self.result_ = Result::Failure;
  }

private:
  char const* name_;
  void (*callback_)(Test&);
  Result result_ = Result::Success;

  inline static thread_local constinit Test* CURRENT_TEST = nullptr;
};

constexpr void fail_quietly() noexcept { Test::current_test().fail_quietly(); }
constexpr void fail(debug::SourceLocation location =
                        debug::SourceLocation::current()) noexcept {
  Test::current_test().fail(location);
}
constexpr void fail(containers::String const& formatted) noexcept {
  Test::current_test().fail(formatted);
}
template <typename... Args>
void fail(fmt::FormatString<Args...> const& fmt, Args&&... args) noexcept {
  Test::current_test().fail(fmt, memory::forward<Args>(args)...);
}

} // namespace yacsl::testing
