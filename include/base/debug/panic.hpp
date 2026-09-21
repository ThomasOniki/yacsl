#pragma once

#include "base/containers/string.hpp"
#include "base/debug/source_location.hpp"
#include "base/format/format_string.hpp"
#include "base/memory/lifetimes.hpp"

namespace base::debug {

[[noreturn]] void
panic(containers::String const& formatted,
      SourceLocation            location = SourceLocation::current()) noexcept;

template <typename... Args>
[[noreturn]] // NOLINTNEXTLINE(misc-no-recursion)
void panic(format::FormatString<Args...> const& fmt, Args&&... args) noexcept {
  containers::String buf;
  format::format_to(buf, fmt, memory::forward<Args>(args)...);
  panic(buf, fmt.source_location());
}

template <typename... Args> // NOLINTNEXTLINE(misc-no-recursion)
void panic_if(bool condition, format::FormatString<Args...> const& fmt,
              Args&&... args) noexcept {
  if (condition) [[unlikely]] {
    panic(fmt, memory::forward<Args>(args)...);
  }
}

#ifdef NDEBUG
template <typename... Args>
[[noreturn]]
void debug_panic(format::FormatString<Args...> const& /*fmt*/,
                 Args&&... /*args*/) noexcept {
  __builtin_unreachable();
}

#else
template <typename... Args>
[[noreturn]]
void debug_panic(format::FormatString<Args...> const& fmt,
                 Args&&... args) noexcept {
  panic(fmt, memory::forward<Args>(args)...);
}

#endif

template <typename... Args>
void debug_panic_if(bool condition, format::FormatString<Args...> const& fmt,
                    Args&&... args) noexcept {
  if (condition) [[unlikely]] {
    debug_panic(fmt, memory::forward<Args>(args)...);
  }
}

} // namespace base::debug
