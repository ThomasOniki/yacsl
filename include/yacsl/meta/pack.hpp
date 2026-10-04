#pragma once

#include "yacsl/meta/same_as.hpp"
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

template <typename T, typename... Ts>
constexpr usize number_in_pack = (... + (SameAs<T, Ts>));

template <typename... Ts>
constexpr bool no_duplicates = (... && (number_in_pack<Ts, Ts...> == 1));

} // namespace yacsl::meta
