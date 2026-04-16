#pragma once

#include "base/type_traits/categories.hpp"
#include "base/type_traits/sign.hpp"

namespace base {

template <Arithmetic T> struct Limits;

template <UnsignedIntegral T>
	requires(Arithmetic<T>)
struct Limits<T> {
	static constexpr T max = T(-1);
	static constexpr T min = 0;
};

template <SignedIntegral T>
	requires(Arithmetic<T>)
struct Limits<T> {
	static constexpr T max = T(Limits<MakeUnsigned<T>>::max / 2 - 1);
	static constexpr T min = T(-Limits<MakeUnsigned<T>>::max / 2);
};

} // namespace base
