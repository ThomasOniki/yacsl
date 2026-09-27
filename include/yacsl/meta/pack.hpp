#pragma once

#include "yacsl/type_aliases.hpp"

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

} // namespace yacsl::meta
