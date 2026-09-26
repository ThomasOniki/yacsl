#pragma once

#include "yacsl/containers/string.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/export.hpp"
#include "yacsl/format/format_string.hpp"
#include "yacsl/memory/lifetimes.hpp"

namespace yacsl::debug {

[[noreturn]] void
panic(containers::String const& formatted,
      SourceLocation location = SourceLocation::current()) noexcept YACSL_API;

template <typename... Args>
[[noreturn]] // NOLINTNEXTLINE(misc-no-recursion)
void panic(format::FormatString<Args...> const& fmt, Args&&... args) noexcept {
  panic(format::format(fmt, memory::forward<Args>(args)...),
        fmt.source_location());
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

} // namespace yacsl::debug
