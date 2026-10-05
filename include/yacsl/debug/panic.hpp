#pragma once

#include "yacsl/containers/string.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/export.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/memory/lifetimes.hpp"

namespace yacsl::debug {

[[noreturn]] void
panic_impl(containers::String const& formatted) noexcept YACSL_API;

[[noreturn]] inline void
panic(SourceLocation location = SourceLocation::current()) noexcept {
  panic_impl(fmt::format("panic at {}\n"
                         "stacktrace:\n{}",
                         location, StackTrace::current()));
}

template <typename... Args>
[[noreturn]]
void panic(fmt::FormatString<Args...> const& fmt, Args&&... args) noexcept {
  panic_impl(fmt::format("panic at {}\n", fmt.source_location()) +
             fmt::format(fmt, memory::forward<Args>(args)...) +
             fmt::format("\nstacktrace:\n{}", StackTrace::current()));
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
  panic_impl(fmt::format("unreachable called at {}\n", fmt.source_location()) +
             fmt::format(fmt, memory::forward<Args>(args)...) +
             fmt::format("\nstacktrace:\n{}", StackTrace::current()));
}

#endif

} // namespace yacsl::debug
