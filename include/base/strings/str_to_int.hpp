#pragma once

#include "base/containers/string_view.hpp"
#include "base/type_traits/categories.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

constexpr const char* num_end(BASE_LIFETIMEBOUND const char* begin) {
  while (*begin >= '0' && *begin <= '9') {
    ++begin;
  }
  return begin;
}

template <Integral T, typename Char = char, typename SizeT = usize>
constexpr T str_to_int(const BasicStringView<Char, SizeT>& str) noexcept {
  T ret = 0;
  for (const char chr : str) {
    ret *= 10;
    ret += static_cast<T>(chr - '0');
  }
  return ret;
}

} // namespace base
