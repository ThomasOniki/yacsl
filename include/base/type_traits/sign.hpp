#pragma once

#include "base/type_traits/categories.hpp"

#if __has_builtin(__make_unsigned) && __has_builtin(__make_signed) && 0
namespace base {
template <Integral T> using MakeUnsigned = __make_unsigned(T);
template <Integral T> using MakeSigned	 = __make_signed(T);
} // namespace base
#else

#include "base/utilities/extended_types.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

namespace internal {

template <usize Target, typename First, typename... Others> struct FirstWithMachingSize;

template <usize Target, typename First, typename... Others>
	requires(sizeof(First) != Target)
struct FirstWithMachingSize<Target, First, Others...> {
	using Type = FirstWithMachingSize<Target, Others...>::Type;
};
template <usize Target, typename First, typename... Others>
	requires(sizeof(First) == Target)
struct FirstWithMachingSize<Target, First, Others...> {
	using Type = First;
};

#ifdef BASE_INT128_FOUND
template <typename T> struct MakeUnsignedImpl {
	using Type = FirstWithMachingSize<sizeof(T), u8, u16, u32, u64, u128>::Type;
};
template <typename T> struct MakeSignedImpl {
	using Type = FirstWithMachingSize<sizeof(T), i8, i16, i32, i64, i128>::Type;
};
#else
template <typename T> struct MakeUnsignedImpl {
	using Type = FirstWithMachingSize<sizeof(T), u8, u16, u32, u64>::Type;
};
template <typename T> struct MakeSignedImpl {
	using Type = FirstWithMachingSize<sizeof(T), i8, i16, i32, i64>::Type;
};
#endif

template <typename T> struct MakeUnsignedImpl<const T> {
	using Type = const MakeUnsignedImpl<T>::Type;
};
template <typename T> struct MakeSignedImpl<const T> {
	using Type = const MakeSignedImpl<T>::Type;
};

// NOLINTBEGIN(google-runtime-int)
// if sizeof(int) == sizeof(long), MakeUnsigned<long> will be unsigned int unless you explicitely
// instantiate.
template <> struct MakeUnsignedImpl<long> {
	using Type = unsigned long;
};
template <> struct MakeUnsignedImpl<unsigned long> {
	using Type = unsigned long;
};
template <> struct MakeSignedImpl<long> {
	using Type = long;
};
template <> struct MakeSignedImpl<unsigned long> {
	using Type = long;
};

// if sizeof(long) == sizeof(long long), MakeUnsigned<long long> will be unsigned long unless you
// explicitely instantiate.
template <> struct MakeUnsignedImpl<long long> {
	using Type = unsigned long long;
};
template <> struct MakeUnsignedImpl<unsigned long long> {
	using Type = unsigned long long;
};
template <> struct MakeSignedImpl<long long> {
	using Type = long long;
};
template <> struct MakeSignedImpl<unsigned long long> {
	using Type = long long;
};
// NOLINTEND(google-runtime-int)

} // namespace internal

template <Integral T> using MakeUnsigned = internal::MakeUnsignedImpl<T>::Type;
template <Integral T> using MakeSigned	 = internal::MakeSignedImpl<T>::Type;

} // namespace base

#endif
