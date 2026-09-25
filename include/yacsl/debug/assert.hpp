#pragma once

#include "yacsl/debug/panic.hpp"
#include "yacsl/format/format_string.hpp"
#include "yacsl/memory/lifetimes.hpp"

namespace yacsl::debug {

template <typename... Args>
constexpr void assert_(bool cond, format::FormatString<Args...> const& fmt,
                       Args&&... args) {
  if (!cond) [[unlikely]] {
    panic(fmt, memory::forward<Args>(args)...);
  }
}

template <typename... Args>
constexpr void assert_unchecked(bool                                 cond,
                                format::FormatString<Args...> const& fmt,
                                Args&&... args) {
  if (!cond) [[unlikely]] {
    unreachable(fmt, memory::forward<Args>(args)...);
  }
}

} // namespace yacsl::debug
