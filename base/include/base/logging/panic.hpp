#pragma once

#include "base/containers/char_buffer.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/common.hpp"
#include "base/logging/format_string.hpp"
#include "base/utilities/move_forward.hpp"

namespace base {

[[noreturn]] void terminate() noexcept;

template <typename... Args>
[[noreturn]]
void panic(const FormatString<Args...>& fmt, Args&&... args) {
  CharBuffer buf;
  buf.append(COLORED_LOG_LEVEL_STRINGS[static_cast<u8>(LogLevel::Panic)]);
  format_to(buf, fmt, base::forward<Args>(args)...);
  terminate();
}
[[noreturn]] inline void panic(StringView msg) {
  CharBuffer buf;
  buf.append(COLORED_LOG_LEVEL_STRINGS[static_cast<u8>(LogLevel::Panic)]);
  buf.append(msg);
  terminate();
}

template <typename... Args>
void panic_if(bool condition, const FormatString<Args...>& fmt, Args&&... args) {
  if (condition) [[unlikely]] {
    panic(fmt, base::forward<Args>(args)...);
  }
}
inline void panic_if(bool condition, StringView msg) {
  if (condition) [[unlikely]] {
    panic(msg);
  }
}

#ifdef NDEBUG
template <typename... Args>
[[noreturn]]
void debug_panic(const FormatString<Args...>& /*fmt*/, Args&&... /*args*/) {
  __builtin_unreachable();
}
[[noreturn]] inline void debug_panic(StringView /*msg*/) { __builtin_unreachable(); }

#else
template <typename... Args>
[[noreturn]]
void debug_panic(const FormatString<Args...>& fmt, Args&&... args) {
  panic(fmt, base::forward<Args>(args)...);
}
[[noreturn]] inline void debug_panic(StringView msg) { panic(msg); }

#endif

template <typename... Args>
void debug_panic_if(bool condition, const FormatString<Args...>& fmt, Args&&... args) {
  if (condition) [[unlikely]] {
    debug_panic(fmt, base::forward<Args>(args)...);
  }
}
inline void debug_panic_if(bool condition, StringView msg) {
  if (condition) [[unlikely]] {
    debug_panic(msg);
  }
}

} // namespace base
