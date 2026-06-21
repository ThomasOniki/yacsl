module;

#include "base/attributes.hpp"

export module base.strings.str_to_int;

import base.ranges.string_view;
import base.utilities.type_aliases;
import base.type_traits.categories;

export namespace base {

constexpr const char* num_end(BASE_LIFETIMEBOUND const char* begin) {
  while (*begin >= '0' && *begin <= '9') {
    ++begin;
  }
  return begin;
}

template <Integral T, typename Char = char, Integral SizeT = usize>
constexpr T str_to_int(BasicStringView<Char, SizeT> str) noexcept {
  T ret = 0;
  for (const char chr : str) {
    ret *= 10;
    ret += static_cast<T>(chr - '0');
  }
  return ret;
}

} // namespace base
