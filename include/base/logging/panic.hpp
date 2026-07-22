#pragma once

#include "base/containers/string.hpp"
#include "base/formatting/format_string.hpp"
#include "base/memory/lifetimes.hpp"

namespace base {

[[noreturn]] void panic(const String& formatted) noexcept;

template <typename... Args>
[[noreturn]] // NOLINTNEXTLINE(misc-no-recursion)
void panic(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  String buf;
  format_to(buf, fmt, base::forward<Args>(args)...);
  panic(buf);
}

template <typename... Args> // NOLINTNEXTLINE(misc-no-recursion)
void panic_if(bool condition, const FormatString<Args...>& fmt, Args&&... args) noexcept {
  if (condition) [[unlikely]] {
    panic(fmt, base::forward<Args>(args)...);
  }
}

#ifdef NDEBUG
template <typename... Args>
[[noreturn]]
void debug_panic(const FormatString<Args...>& /*fmt*/, Args&&... /*args*/) noexcept {
  __builtin_unreachable();
}

#else
template <typename... Args>
[[noreturn]]
void debug_panic(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  panic(fmt, base::forward<Args>(args)...);
}

#endif

template <typename... Args>
void debug_panic_if(bool condition, const FormatString<Args...>& fmt, Args&&... args) noexcept {
  if (condition) [[unlikely]] {
    debug_panic(fmt, base::forward<Args>(args)...);
  }
}

} // namespace base
