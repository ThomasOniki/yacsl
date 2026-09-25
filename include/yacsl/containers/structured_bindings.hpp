#pragma once

#ifndef YACSL_NO_LIBCPP
#include <utility>
#endif

#include "yacsl/type_aliases.hpp"

namespace yacsl::containers {

template <typename T> struct TupleSize;
template <typename T, usize I> struct TupleElement;

} // namespace yacsl::containers

#ifdef YACSL_NO_LIBCPP
namespace std {
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
template <typename T> struct tuple_size;
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
template <yacsl::usize I, typename T> struct tuple_element;
} // namespace std
#endif

namespace std {
template <typename T>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
  requires requires() {
    { yacsl::containers::TupleSize<T>::value };
  }
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct tuple_size<T> {
  static constexpr yacsl::usize value = yacsl::containers::TupleSize<T>::value;
};
template <yacsl::usize I, typename T>
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
  requires requires() {
    { yacsl::containers::TupleSize<T>::value };
  }
// NOLINTNEXTLINE(bugprone-std-namespace-modification)
struct tuple_element<I, T> {
  using type = yacsl::containers::TupleElement<T, I>::Type;
};
} // namespace std
