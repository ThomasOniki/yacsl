#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::fmt {

template <meta::Character T>
constexpr T const* num_end(YACSL_LIFETIMEBOUND T const* begin) {
  while (*begin >= '0' && *begin <= '9') {
    ++begin;
  }
  return begin;
}

template <meta::Integral T, meta::Character C = char,
          meta::Integral SizeT = usize>
constexpr T atoi(ranges::GenericStringView<C, SizeT> str) noexcept {
  T ret = 0;
  for (char const chr : str) {
    ret *= 10;
    ret += static_cast<T>(chr - '0');
  }
  return ret;
}
template <meta::Integral T, meta::Character C = char,
          meta::Integral SizeT = usize>
constexpr T atoi(C const* begin, C const* end) {
  return fmt::atoi<T>(ranges::GenericStringView<C, SizeT>(begin, end));
}
template <meta::Integral T, meta::Character C = char,
          meta::Integral SizeT = usize>
constexpr T atoi(C const* ptr, SizeT len) {
  return fmt::atoi<T>(ranges::GenericStringView<C, SizeT>(ptr, len));
}

} // namespace yacsl::fmt
