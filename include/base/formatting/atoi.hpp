#pragma once

#include "base/attributes.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/categories.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <Character Char> constexpr const Char* num_end(BASE_LIFETIMEBOUND const Char* begin) {
  while (*begin >= '0' && *begin <= '9') {
    ++begin;
  }
  return begin;
}

template <Integral T, Character Char = char, Integral SizeT = usize>
constexpr T atoi(const Char* begin, const Char* end) {
  return atoi<T>(BasicStringView<Char, SizeT>(begin, end));
}
template <Integral T, Character Char = char, Integral SizeT = usize>
constexpr T atoi(const Char* ptr, SizeT len) {
  return atoi<T>(BasicStringView<Char, SizeT>(ptr, len));
}

template <Integral T, Character Char = char, Integral SizeT = usize>
constexpr T atoi(BasicStringView<Char, SizeT> str) noexcept {
  T ret = 0;
  for (const char chr : str) {
    ret *= 10;
    ret += static_cast<T>(chr - '0');
  }
  return ret;
}

} // namespace base
