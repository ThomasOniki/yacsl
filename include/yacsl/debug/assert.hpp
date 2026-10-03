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

} // namespace yacsl::debug
