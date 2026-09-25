#pragma once

#include "yacsl/type_traits/comparaison.hpp"
#include "yacsl/type_traits/qualifiers.hpp"

namespace yacsl::type_traits {

#if __has_builtin(__is_integral)
template <typename T>
concept Integral = __is_integral(T);
#else
template <typename T>
#if defined(__SIZEOF_INT128__)
concept Integral =
    OneOf<RemoveCV<T>, bool, char, signed char, unsigned char, wchar_t, char8_t,
          char16_t, char32_t, short, unsigned short, int, unsigned int, long,
          unsigned long, long long, unsigned long long, __int128_t,
          __uint128_t>;
#else
concept Integral =
    OneOf<RemoveCV<T>, bool, char, signed char, unsigned char, wchar_t, char8_t,
          char16_t, char32_t, short, unsigned short, int, unsigned int, long,
          unsigned long, long long, unsigned long long>;
#endif
#endif

#if __has_builtin(__is_floating_point)
template <typename T>
concept FloatingPoint = __is_floating_point(T);
#else
template <typename T>
concept FloatingPoint = OneOf<RemoveCV<T>, float, double, long double>;
#endif

#if __has_builtin(__is_arithmetic)
template <typename T>
concept Arithmetic = __is_arithmetic(T);
#else
template <typename T>
concept Arithmetic = FloatingPoint<T> || Integral<T>;
#endif

template <typename T>
concept Character = OneOf<RemoveCV<T>, char, signed char, unsigned char,
                          wchar_t, char8_t, char16_t, char32_t>;

template <typename T>
concept SignedIntegral = Integral<T> && T(-1) < T(0);
template <typename T>
concept UnsignedIntegral = Integral<T> && T(-1) > T(0);

template <typename T>
concept Enum = __is_enum(T);

} // namespace yacsl::type_traits
