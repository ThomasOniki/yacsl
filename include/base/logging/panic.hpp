#pragma once

#include "base/containers/string.hpp"
#include "base/formatting/format_string.hpp"
#include "base/logging/common.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

[[noreturn]] void panic(const String& formatted) noexcept;

template <typename... Args>
[[noreturn]]
void panic(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  String buf;
  format_to(buf, fmt, base::forward<Args>(args)...);
  panic(buf);
}

template <typename... Args>
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
