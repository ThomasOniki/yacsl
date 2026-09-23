#pragma once

#include "base/debug/panic.hpp"
#include "base/format/format_string.hpp"
#include "base/memory/lifetimes.hpp"

namespace base::debug {

template <typename... Args>
constexpr void assert_true(bool cond, format::FormatString<Args...> const& fmt,
                           Args&&... args) {
  if (!cond) [[unlikely]] {
    panic(fmt, memory::forward<Args>(args)...);
  }
}

template <typename... Args>
constexpr void assert_true_unchecked(bool                                 cond,
                                     format::FormatString<Args...> const& fmt,
                                     Args&&... args) {
  if (!cond) [[unlikely]] {
    unreachable(fmt, memory::forward<Args>(args)...);
  }
}

} // namespace base::debug
