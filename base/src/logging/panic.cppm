export module base.logging.panic;

import base.logging.format_string;
import base.logging.common;
import base.ranges.string_view;
import base.memory.lifetimes;
import base.utilities.type_aliases;
import base.containers.string;

export namespace base {

[[noreturn]] void panic(const String& formatted) noexcept;

template <typename... Args>
[[noreturn]]
void panic(const FormatString<Args...>& fmt, Args&&... args) noexcept {
  String buf;
  buf += LOG_LEVEL_STRINGS[static_cast<u8>(LogLevel::Panic)];
  format_to(buf, fmt, base::forward<Args>(args)...);
  buf += '\n';
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
