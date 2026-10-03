#pragma once

#include "yacsl/type_aliases.hpp"

namespace yacsl::meta {

namespace internal {

template <usize Size, typename First, typename... Others>
struct FirstOfSizeImpl;

template <usize Size, typename First, typename... Others>
  requires(sizeof(First) != Size)
struct FirstOfSizeImpl<Size, First, Others...> {
  using Type = FirstOfSizeImpl<Size, Others...>::Type;
};
template <usize Size, typename First, typename... Others>
  requires(sizeof(First) == Size)
struct FirstOfSizeImpl<Size, First, Others...> {
  using Type = First;
};

} // namespace internal

template <usize Size, typename... Ts>
using FirstOfSize = internal::FirstOfSizeImpl<Size, Ts...>;

} // namespace yacsl::meta
