export module base.math.powers;

import base.utilities.type_aliases;
import base.type_traits.categories;

#ifdef BASE_COMPILER_MSVC
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#endif

export namespace base {

template <Integral T> constexpr T pow10(u32 exp) noexcept {
  T ret = 1;
  for (u32 i = 0; i < exp; ++i) {
    ret = static_cast<T>(ret * 10);
  }
  return ret;
}

template <Integral T> constexpr T pow2(u32 exp) noexcept { return 1 << exp; }
template <Integral T> constexpr T pow4(u32 exp) noexcept { return 1 << 2 * exp; }
template <Integral T> constexpr T pow8(u32 exp) noexcept { return 1 << 3 * exp; }
template <Integral T> constexpr T pow16(u32 exp) noexcept { return 1 << 4 * exp; }

} // namespace base
