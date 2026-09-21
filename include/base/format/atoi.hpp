#pragma once

#include "base/attributes.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_aliases.hpp"
#include "base/type_traits/categories.hpp"

namespace base::format {

template <type_traits::Character Char>
constexpr Char const* num_end(BASE_LIFETIMEBOUND Char const* begin) {
  while (*begin >= '0' && *begin <= '9') {
    ++begin;
  }
  return begin;
}

template <type_traits::Integral T, type_traits::Character Char = char,
          type_traits::Integral SizeT = usize>
constexpr T atoi(ranges::BasicStringView<Char, SizeT> str) noexcept {
  T ret = 0;
  for (char const chr : str) {
    ret *= 10;
    ret += static_cast<T>(chr - '0');
  }
  return ret;
}
template <type_traits::Integral T, type_traits::Character Char = char,
          type_traits::Integral SizeT = usize>
constexpr T atoi(Char const* begin, Char const* end) {
  return format::atoi<T>(ranges::BasicStringView<Char, SizeT>(begin, end));
}
template <type_traits::Integral T, type_traits::Character Char = char,
          type_traits::Integral SizeT = usize>
constexpr T atoi(Char const* ptr, SizeT len) {
  return format::atoi<T>(ranges::BasicStringView<Char, SizeT>(ptr, len));
}

} // namespace base::format
