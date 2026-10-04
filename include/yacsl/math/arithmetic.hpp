#pragma once

#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/same_as.hpp"
#include "yacsl/type_aliases.hpp"

#ifdef YACSL_COMPILER_MSVC
#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#endif

namespace yacsl::math {

template <meta::Integral T> constexpr T pow10(u32 exp) noexcept {
  T ret = 1;
  for (u32 i = 0; i < exp; ++i) {
    ret = static_cast<T>(ret * 10);
  }
  return ret;
}

template <meta::Integral T> constexpr T pow2(u32 exp) noexcept {
  return 1U << exp;
}
template <meta::Integral T> constexpr T pow4(u32 exp) noexcept {
  return 1U << 2 * exp;
}
template <meta::Integral T> constexpr T pow8(u32 exp) noexcept {
  return 1U << 3 * exp;
}
template <meta::Integral T> constexpr T pow16(u32 exp) noexcept {
  return 1U << 4 * exp;
}

template <typename T> constexpr T&& max(T&& lhs, T&& rhs) noexcept {
  return memory::forward<T>(lhs) > memory::forward<T>(rhs)
             ? memory::forward<T>(lhs)
             : memory::forward<T>(rhs);
}
template <typename T, typename... Others>
constexpr T&& max(T&& first, Others&&... others)
  requires(... && meta::SameAs<T, Others>)
{
  return max(memory::forward<T>(first),
             max(memory::forward<Others>(others)...));
}

template <typename T> constexpr T&& min(T&& lhs, T&& rhs) noexcept {
  return memory::forward<T>(lhs) < memory::forward<T>(rhs)
             ? memory::forward<T>(lhs)
             : memory::forward<T>(rhs);
}
template <typename T, typename... Others>
constexpr T&& min(T&& first, Others&&... others)
  requires(... && meta::SameAs<T, Others>)
{
  return min(memory::forward<T>(first),
             min(memory::forward<Others>(others)...));
}

} // namespace yacsl::math
