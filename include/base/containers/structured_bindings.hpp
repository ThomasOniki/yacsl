#pragma once

#ifndef BASE_NO_LIBCPP
#include <utility>
#endif

#include "base/type_aliases.hpp"

namespace base::containers {

template <typename T> struct TupleSize;
template <typename T, usize I> struct TupleElement;

} // namespace base::containers

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
    { base::containers::TupleSize<T>::value };
  }
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct tuple_size<T> {
  static constexpr base::usize value = base::containers::TupleSize<T>::value;
};
template <base::usize I, typename T>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
  requires requires() {
    { base::containers::TupleSize<T>::value };
  }
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct tuple_element<I, T> {
  using type = base::containers::TupleElement<T, I>::Type;
};
} // namespace std
