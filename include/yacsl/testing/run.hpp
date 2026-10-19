#pragma once

#include "yacsl/containers/string.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/ranges/string_view.hpp"

namespace yacsl::testing {

constexpr bool*& status_ptr() noexcept {
  static thread_local bool  default_result_variable = true;
  static thread_local bool* result_ptr              = &default_result_variable;
  return result_ptr;
}
constexpr bool& status() noexcept { return *status_ptr(); }

template <typename F, typename... Args>
constexpr bool run_quitely(F&& func, Args&&... args) noexcept {
  bool* last_ptr = status_ptr();

  bool current_status = true;
  status_ptr()        = &current_status;
  memory::forward<F>(func)(memory::forward<Args>(args)...);

  status_ptr() = last_ptr;
  status()     = status() && current_status;
  return current_status;
}

template <typename F, typename... Args>
constexpr bool run(ranges::StringView name, F&& func, Args&&... args) noexcept {
  debug::log_info("Running test {?}...", name);

  if (run_quitely(memory::forward<F>(func), memory::forward<Args>(args)...)) {
    debug::log_info("Test {?} suceeded!", name);
    return true;
  }
  debug::log_error("Test {?} failed!", name);
  return false;
}

constexpr void fail_quietly() noexcept { status() = false; }
constexpr void fail(debug::SourceLocation location =
                        debug::SourceLocation::current()) noexcept {
  debug::log_error("Test failed at {}.", location);
  status() = false;
}
constexpr void fail(containers::String const& formatted) noexcept {
  debug::log_error(formatted);
  status() = false;
}
template <typename... Args>
void fail(fmt::FormatString<Args...> const& fmt, Args&&... args) noexcept {
  debug::log_error(fmt::format("Test failed at {}.\n", fmt.source_location()) +
                   fmt::format(fmt, memory::forward<Args>(args)...));
  status() = false;
}

} // namespace yacsl::testing
