#pragma once

#include "base/type_aliases.hpp"

namespace base::meta::internal {
template <bool Cond, typename If, typename Else> struct ConditionalTypeImpl {
  using Type = If;
};
template <typename If, typename Else>
struct ConditionalTypeImpl<false, If, Else> {
  using Type = Else;
};
template <usize N, typename First, typename... Others> struct NthTypeImpl {
  using Type = NthTypeImpl<N - 1, Others...>::Type;
};
template <typename First, typename... Others>
struct NthTypeImpl<0, First, Others...> {
  using Type = First;
};
} // namespace base::meta::internal

namespace base::meta {

template <bool Cond, typename If, typename Else>
using ConditionalT = internal::ConditionalTypeImpl<Cond, If, Else>::Type;
template <usize N, typename... Ts>
using NthType = internal::NthTypeImpl<N, Ts...>::Type;

template <typename... Ts> using LastType = NthType<sizeof...(Ts) - 1, Ts...>;

} // namespace base::meta
