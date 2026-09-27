#pragma once

#include "yacsl/containers/string.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/export.hpp"
#include "yacsl/format/format_string.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include <stdlib.h>

namespace yacsl::debug {

[[noreturn]] void
panic_impl(containers::String const& formatted) noexcept YACSL_API;

[[noreturn]] inline void
panic(SourceLocation location = SourceLocation::current()) noexcept {
  panic_impl(format::format("panic at {}\nstacktrace: {}", location,
                            StackTrace::current()));
}

template <typename... Args>
[[noreturn]] // NOLINTNEXTLINE(misc-no-recursion)
void panic(format::FormatString<Args...> const& fmt, Args&&... args) noexcept {
  panic_impl(format::format("panic at {}\n", fmt.source_location()) +
             format::format(fmt, memory::forward<Args>(args)...) +
             format::format("\nstacktrace: {}", StackTrace::current()));
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
