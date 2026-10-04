#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::fmt {

template <meta::Character Char>
constexpr Char const* num_end(YACSL_LIFETIMEBOUND Char const* begin) {
  while (*begin >= '0' && *begin <= '9') {
    ++begin;
  }
  return begin;
}

template <meta::Integral T, meta::Character Char = char,
          meta::Integral SizeT = usize>
constexpr T atoi(ranges::BasicStringView<Char, SizeT> str) noexcept {
  T ret = 0;
  for (char const chr : str) {
    ret *= 10;
    ret += static_cast<T>(chr - '0');
  }
  return ret;
}
template <meta::Integral T, meta::Character Char = char,
          meta::Integral SizeT = usize>
constexpr T atoi(Char const* begin, Char const* end) {
  return fmt::atoi<T>(ranges::BasicStringView<Char, SizeT>(begin, end));
}
template <meta::Integral T, meta::Character Char = char,
          meta::Integral SizeT = usize>
constexpr T atoi(Char const* ptr, SizeT len) {
  return fmt::atoi<T>(ranges::BasicStringView<Char, SizeT>(ptr, len));
}

} // namespace yacsl::fmt
