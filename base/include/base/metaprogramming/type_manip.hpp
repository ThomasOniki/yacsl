#pragma once

#include "base/utilities/type_aliases.hpp"

namespace base {

namespace internal {
template <bool Cond, typename If, typename Else> struct ConditionalTypeImpl {
  using Type = If;
};
template <typename If, typename Else> struct ConditionalTypeImpl<false, If, Else> {
  using Type = Else;
};
template <usize N, typename First, typename... Others> struct NthTypeImpl {
  using Type = NthTypeImpl<N - 1, Others...>::Type;
};
template <typename First, typename... Others> struct NthTypeImpl<0, First, Others...> {
  using Type = First;
};
} // namespace internal

template <bool Cond, typename If, typename Else>
using ConditionalT = internal::ConditionalTypeImpl<Cond, If, Else>::Type;
template <usize N, typename... Ts> using NthType = internal::NthTypeImpl<N, Ts...>::Type;

template <typename... Ts> using LastType = NthType<sizeof...(Ts) - 1, Ts...>;

} // namespace base
