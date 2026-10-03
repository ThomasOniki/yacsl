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
    panic_impl(fmt::format("assert failed at {}\n", location) +
               fmt::format("stacktrace:\n{}", StackTrace::current()));
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

#ifdef NDEBUG
constexpr void
assert_unchecked(bool cond,
                 SourceLocation /*location*/ = SourceLocation::current()) {
  if (!cond) [[unlikely]] {
    __builtin_unreachable();
  }
}
#else
constexpr void
assert_unchecked(bool           cond,
                 SourceLocation location = SourceLocation::current()) {
  if (!cond) [[unlikely]] {
    panic_impl(fmt::format("assert failed at {}\n", location) +
               fmt::format("\nstacktrace:\n{}", StackTrace::current()));
  }
}
#endif

#ifdef NDEBUG
template <typename... Args>
constexpr void assert_unchecked(bool cond,
                                fmt::FormatString<Args...> const& /*fmt*/,
                                Args&&... /*args*/) {
  if (!cond) [[unlikely]] {
    __builtin_unreachable();
  }
}
#else
template <typename... Args>
constexpr void assert_unchecked(bool                              cond,
                                fmt::FormatString<Args...> const& fmt,
                                Args&&... args) {
  if (!cond) [[unlikely]] {
    unreachable_impl(
        fmt::format("assert failed at {}\n", fmt.source_location()) +
        fmt::format(fmt, memory::forward<Args>(args)...) +
        fmt::format("\nstacktrace:\n{}", StackTrace::current()));
  }
}
#endif

} // namespace yacsl::debug
