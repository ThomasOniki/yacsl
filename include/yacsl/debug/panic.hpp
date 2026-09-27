#pragma once

#include "yacsl/containers/string.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/export.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include <stdlib.h>

namespace yacsl::debug {

[[noreturn]] void
panic_impl(containers::String const& formatted) noexcept YACSL_API;

[[noreturn]] inline void
panic(SourceLocation location = SourceLocation::current()) noexcept {
  panic_impl(fmt::format("panic at {}\nstacktrace: {}", location,
                            StackTrace::current()));
}

template <typename... Args>
[[noreturn]] // NOLINTNEXTLINE(misc-no-recursion)
void panic(fmt::FormatString<Args...> const& fmt, Args&&... args) noexcept {
  panic_impl(fmt::format("panic at {}\n", fmt.source_location()) +
             fmt::format(fmt, memory::forward<Args>(args)...) +
             fmt::format("\nstacktrace: {}", StackTrace::current()));
}

#ifdef NDEBUG
template <typename... Args>
[[noreturn]]
void unreachable(fmt::FormatString<Args...> const& /*fmt*/,
                 Args&&... /*args*/) noexcept {
  __builtin_unreachable();
}

#else
template <typename... Args>
[[noreturn]]
void unreachable(fmt::FormatString<Args...> const& fmt,
                 Args&&... args) noexcept {
  panic(fmt, memory::forward<Args>(args)...);
}

#endif

} // namespace yacsl::debug
