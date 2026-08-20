#pragma once

#include "base/type_traits/categories.hpp"

#ifdef __clang__
namespace base {
template <Integral T> using MakeUnsigned = __make_unsigned(T);
template <Integral T> using MakeSigned   = __make_signed(T);
} // namespace base
#else

#include "base/metaprogramming/first_of_size.hpp"
#include "base/utilities/extended_types.hpp"

namespace base::internal {

template <typename T> struct MakeUnsignedImpl {
  using Type = FirstOfSize<sizeof(T), unsigned char, unsigned short, unsigned int, unsigned long,
                           unsigned long long, u128>::Type;
};
template <typename T> struct MakeSignedImpl {
  using Type = FirstOfSize<sizeof(T), signed char, short, int, long, long long, i128>::Type;
};

template <typename T> struct MakeUnsignedImpl<const T> {
  using Type = const MakeUnsignedImpl<T>::Type;
};
template <typename T> struct MakeSignedImpl<const T> {
  using Type = const MakeSignedImpl<T>::Type;
};

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

} // namespace base::internal

namespace base {

template <Integral T> using MakeUnsigned = internal::MakeUnsignedImpl<T>::Type;
template <Integral T> using MakeSigned   = internal::MakeSignedImpl<T>::Type;

} // namespace base

#endif
