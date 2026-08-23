#pragma once

#include "base/type_traits/categories.hpp"
#include "base/type_traits/sign.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <Arithmetic T> struct Limits;

template <UnsignedIntegral T>
  requires(Arithmetic<T>)
struct Limits<T> {
  static constexpr T   MAX      = T(-1);
  static constexpr T   MIN      = 0;
  static constexpr u32 MAX_BITS = sizeof(T) * 8;
};

template <SignedIntegral T>
  requires(Arithmetic<T>)
struct Limits<T> {
  static constexpr T   MAX      = T(Limits<MakeUnsigned<T>>::MAX / 2 - 1);
  static constexpr T   MIN      = -MAX - 1;
  static constexpr u32 MAX_BITS = sizeof(T) * 8;
};

} // namespace base
