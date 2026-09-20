#pragma once

#ifndef BASE_NO_LIBCPP
#include <utility>
#endif

#include "base/type_aliases.hpp"

namespace base {

template <typename T> struct TupleSize;
template <typename T, usize I> struct TupleElement;

} // namespace base

#ifdef BASE_NO_LIBCPP
namespace std {
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
template <typename T> struct tuple_size;
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
template <base::usize I, typename T> struct tuple_element;
} // namespace std
#endif

namespace std {
template <typename T>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
  requires requires() {
    { base::TupleSize<T>::value };
  }
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct tuple_size<T> {
  static constexpr base::usize value = base::TupleSize<T>::value;
};
template <base::usize I, typename T>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
  requires requires() {
    { base::TupleSize<T>::value };
  }
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct tuple_element<I, T> {
  using type = base::TupleElement<T, I>::Type;
};
} // namespace std
