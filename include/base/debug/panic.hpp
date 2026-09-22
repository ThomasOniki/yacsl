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

#ifdef NDEBUG
template <typename... Args>
[[noreturn]]
void unreachable(format::FormatString<Args...> const& /*fmt*/,
                 Args&&... /*args*/) noexcept {
  __builtin_unreachable();
}

#else
template <typename... Args>
[[noreturn]]
void unreachable(format::FormatString<Args...> const& fmt,
                 Args&&... args) noexcept {
  panic(fmt, memory::forward<Args>(args)...);
}

#endif

} // namespace base::debug
