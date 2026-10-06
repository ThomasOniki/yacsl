#pragma once

#include "yacsl/debug/source_location.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/testing/test.hpp"

namespace yacsl::testing {

inline void
expect(bool                  cond,
       debug::SourceLocation location = debug::SourceLocation::current()) {
  if (!cond) [[unlikely]] {
    fail(fmt::format("expect failed in test {?} at {}\n"
                     "stacktrace:\n{}",
                     Test::current_test().name(), location,
                     debug::StackTrace::current()));
  }
}
template <typename... Args>
void expect(bool cond, fmt::FormatString<Args...> const& fmt, Args&&... args) {
  if (!cond) [[unlikely]] {
    fail(fmt::format("expect failed in test {?} at {}\n",
                     Test::current_test().name(), fmt.source_location()) +
         fmt::format(fmt, memory::forward<Args>(args)...) +
         fmt::format("\nstacktrace:\n{}", debug::StackTrace::current()));
  }
}

template <typename T, typename U>
constexpr void expect_eq(T&& lhs, U&& rhs,
                         debug::SourceLocation location =
                             debug::SourceLocation::current()) noexcept {
  if (memory::forward<T>(lhs) != memory::forward<T>(rhs)) [[unlikely]] {
    fail(fmt::format("expect_eq failed in test {?} at {}\n"
                     "lhs: {}\n"
                     "rhs: {}\n"
                     "stacktrace:\n{}",
                     Test::current_test().name(), location,
                     memory::forward<T>(lhs), memory::forward<U>(rhs),
                     debug::StackTrace::current()));
  }
}
template <typename T, typename U, typename... Args>
constexpr void expect_eq(T&& lhs, U&& rhs, fmt::FormatString<Args...> fmt,
                         Args&&... args) noexcept {
  if (memory::forward<T>(lhs) != memory::forward<T>(rhs)) [[unlikely]] {
    fail(fmt::format("expect_eq failed in test {?} at {}\n"
                     "lhs: {}\n"
                     "rhs: {}\n",
                     Test::current_test().name(), fmt.source_location(),
                     memory::forward<T>(lhs), memory::forward<U>(rhs)) +
         fmt::format(fmt, memory::forward<Args>(args)...) +
         fmt::format("\nstacktrace:\n{}", debug::StackTrace::current()));
  }
}

} // namespace yacsl::testing
