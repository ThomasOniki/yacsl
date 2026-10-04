#pragma once

#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/sign.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::meta {

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

} // namespace yacsl::meta
