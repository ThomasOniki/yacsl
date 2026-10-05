#pragma once

#include "yacsl/debug/panic.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/memory/lifetimes.hpp"

namespace yacsl::debug {

constexpr void assert_(bool           cond,
                       SourceLocation location = SourceLocation::current()) {
  if (!cond) [[unlikely]] {
    panic_impl(fmt::format("assert failed at {}\n"
                           "stacktrace:\n{}",
                           location, StackTrace::current()));
  }
}
template <typename... Args>
constexpr void assert_(bool cond, fmt::FormatString<Args...> const& fmt,
                       Args&&... args) {
  if (!cond) [[unlikely]] {
    panic_impl(fmt::format("assert failed at {}\n", fmt.source_location()) +
               fmt::format(fmt, memory::forward<Args>(args)...) +
               fmt::format("\nstacktrace:\n{}", StackTrace::current()));
  }
}

template <typename T, typename U>
constexpr void
assert_eq(T&& lhs, U&& rhs,
          SourceLocation location = SourceLocation::current()) noexcept {
  if (memory::forward<T>(lhs) != memory::forward<T>(rhs)) [[unlikely]] {
    panic_impl(fmt::format("assert_eq failed at {}\n"
                           "lhs: {}\n"
                           "rhs: {}\n"
                           "stacktrace:\n{}",
                           location, memory::forward<T>(lhs),
                           memory::forward<U>(rhs), StackTrace::current()));
  }
}
template <typename T, typename U, typename... Args>
constexpr void assert_eq(T&& lhs, U&& rhs, fmt::FormatString<Args...> fmt,
                         Args&&... args) noexcept {
  if (memory::forward<T>(lhs) != memory::forward<T>(rhs)) [[unlikely]] {
    panic_impl(fmt::format("assert_eq failed at {}\n"
                           "lhs: {}\n"
                           "rhs: {}\n",
                           fmt.source_location(), memory::forward<T>(lhs),
                           memory::forward<U>(rhs)) +
               fmt::format(fmt, memory::forward<Args>(args)...) +
               fmt::format("\nstacktrace:\n{}", StackTrace::current()));
  }
}

#ifdef NDEBUG
constexpr void assume_ifdef_ndebug(bool cond) noexcept {
  if (!cond) {
    __builtin_unreachable();
  }
}
#else
constexpr void assume_ifdef_ndebug(bool /*cond*/) noexcept {}
#endif

constexpr void
assert_unchecked(bool           cond,
                 SourceLocation location = SourceLocation::current()) {
  assume_ifdef_ndebug(cond);
  assert_(cond, location);
}
template <typename... Args>
constexpr void assert_unchecked(bool                              cond,
                                fmt::FormatString<Args...> const& fmt,
                                Args&&... args) {
  assume_ifdef_ndebug(cond);
  assert_(cond, fmt, memory::forward<Args>(args)...);
}

template <typename T, typename U>
constexpr void assert_eq_unchecked(T&& lhs, U&& rhs) noexcept {
  assume_ifdef_ndebug(memory::forward<T>(lhs) == memory::forward<T>(rhs));
  assert_eq(memory::forward<T>(lhs), memory::forward<T>(rhs));
}
template <typename T, typename U, typename... Args>
constexpr void assert_eq_unchecked(T&& lhs, U&& rhs,
                                   fmt::FormatString<Args...> fmt,
                                   Args&&... args) noexcept {
  assume_ifdef_ndebug(memory::forward<T>(lhs) == memory::forward<T>(rhs));
  assert_eq(memory::forward<T>(lhs), memory::forward<T>(rhs), fmt,
            memory::forward<Args>(args)...);
}

} // namespace yacsl::debug
