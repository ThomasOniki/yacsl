#pragma once

#include "base/containers/string.hpp"
#include "base/debug/source_location.hpp"
#include "base/format/format_string.hpp"
#include "base/memory/lifetimes.hpp"

namespace base {

[[noreturn]] void
panic(String const&  formatted,
      SourceLocation location = SourceLocation::current()) noexcept;

template <typename... Args>
[[noreturn]] // NOLINTNEXTLINE(misc-no-recursion)
void panic(FormatString<Args...> const& fmt, Args&&... args) noexcept {
  String buf;
  format_to(buf, fmt, base::forward<Args>(args)...);
  panic(buf, fmt.source_location());
}

template <typename... Args> // NOLINTNEXTLINE(misc-no-recursion)
void panic_if(bool condition, FormatString<Args...> const& fmt,
              Args&&... args) noexcept {
  if (condition) [[unlikely]] {
    panic(fmt, base::forward<Args>(args)...);
  }
}

#ifdef NDEBUG
template <typename... Args>
[[noreturn]]
void debug_panic(FormatString<Args...> const& /*fmt*/,
                 Args&&... /*args*/) noexcept {
  __builtin_unreachable();
}

#else
template <typename... Args>
[[noreturn]]
void debug_panic(FormatString<Args...> const& fmt, Args&&... args) noexcept {
  panic(fmt, base::forward<Args>(args)...);
}

#endif

template <typename... Args>
void debug_panic_if(bool condition, FormatString<Args...> const& fmt,
                    Args&&... args) noexcept {
  if (condition) [[unlikely]] {
    debug_panic(fmt, base::forward<Args>(args)...);
  }
}

} // namespace base
