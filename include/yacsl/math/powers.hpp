#pragma once

#include "yacsl/type_aliases.hpp"
#include "yacsl/type_traits/categories.hpp"

#ifdef YACSL_COMPILER_MSVC
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#endif

namespace yacsl::math {

template <type_traits::Integral T> constexpr T pow10(u32 exp) noexcept {
  T ret = 1;
  for (u32 i = 0; i < exp; ++i) {
    ret = static_cast<T>(ret * 10);
  }
  return ret;
}

template <type_traits::Integral T> constexpr T pow2(u32 exp) noexcept {
  return 1U << exp;
}
template <type_traits::Integral T> constexpr T pow4(u32 exp) noexcept {
  return 1U << 2 * exp;
}
template <type_traits::Integral T> constexpr T pow8(u32 exp) noexcept {
  return 1U << 3 * exp;
}
template <type_traits::Integral T> constexpr T pow16(u32 exp) noexcept {
  return 1U << 4 * exp;
}

} // namespace yacsl::math
