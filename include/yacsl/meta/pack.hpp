#pragma once

#include "yacsl/type_aliases.hpp"
#include "yacsl/type_traits/comparaison.hpp"

namespace yacsl::meta {

namespace internal {
template <usize N, typename First, typename... Others> struct NthTypeImpl {
  using Type = NthTypeImpl<N - 1, Others...>::Type;
};
template <typename First, typename... Others>
struct NthTypeImpl<0, First, Others...> {
  using Type = First;
};
} // namespace internal

template <usize N, typename... Ts>
using NthType = internal::NthTypeImpl<N, Ts...>::Type;

template <typename... Ts> using LastType = NthType<sizeof...(Ts) - 1, Ts...>;

template <typename T, typename... Ts>
constexpr usize number_in_pack = (... + (type_traits::SameAs<T, Ts>));

template <typename... Ts>
constexpr bool no_duplicates = (... && (number_in_pack<Ts, Ts...> == 1));

} // namespace yacsl::meta
