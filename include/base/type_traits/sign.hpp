#pragma once

#include "base/type_traits/categories.hpp"

#ifdef __clang__
namespace base::type_traits {
template <Integral T> using MakeUnsigned = __make_unsigned(T);
template <Integral T> using MakeSigned   = __make_signed(T);
} // namespace base::type_traits
#else

#include "base/meta/first_of_size.hpp"
#include "base/utils/extended_types.hpp"

namespace base::type_traits {
namespace internal {

template <typename T> struct MakeUnsignedImpl {
  using Type =
      meta::FirstOfSize<sizeof(T), unsigned char, unsigned short, unsigned int,
                        unsigned long, unsigned long long, utils::u128>::Type;
};
template <typename T> struct MakeSignedImpl {
  using Type = meta::FirstOfSize<sizeof(T), signed char, short, int, long,
                                 long long, utils::i128>::Type;
};

template <typename T> struct MakeUnsignedImpl<T const> {
  using Type = MakeUnsignedImpl<T>::Type const;
};
template <typename T> struct MakeSignedImpl<T const> {
  using Type = MakeSignedImpl<T>::Type const;
};

// if sizeof(int) == sizeof(long), MakeUnsigned<long> will be unsigned int
// unless you explicitely instantiate.
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

// if sizeof(long) == sizeof(long long), MakeUnsigned<long long> will be
// unsigned long unless you explicitely instantiate.
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

} // namespace internal

template <Integral T> using MakeUnsigned = internal::MakeUnsignedImpl<T>::Type;
template <Integral T> using MakeSigned   = internal::MakeSignedImpl<T>::Type;

} // namespace base::type_traits

#endif
