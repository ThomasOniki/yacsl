#pragma once

#include "base/type_traits/categories.hpp"
#include "base/utilities/type_aliases.hpp"

#ifdef BASE_COMPILER_MSVC
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#endif

namespace base {

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

template <UnsignedIntegral T> constexpr u32 log2(T value) noexcept {
#ifdef BASE_COMPILER_MSVC
  unsigned long ret;
  _BitScanReverse(&ret, value);
  return static_cast<u32>(value);
#else
  return sizeof(T) * 8 - 1 - static_cast<u32>(__builtin_clz(value));
#endif
}

} // namespace base
