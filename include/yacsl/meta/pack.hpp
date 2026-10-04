#pragma once

#include "yacsl/meta/same_as.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::meta {

template <typename T, typename... Ts>
constexpr usize number_in_pack = (... + (SameAs<T, Ts>));
template <typename... Ts>
constexpr bool no_duplicates = (... && (number_in_pack<Ts, Ts...> == 1));

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

template <usize I, typename T, typename... Ts> struct FirstTypeIndex {
  static constexpr usize value =
      SameAs<T, Ts...[I]> ? I : FirstTypeIndex<I + 1, T, Ts...>::value;
};
template <typename T, typename... Ts>
struct FirstTypeIndex<sizeof...(Ts), T, Ts...> {
  static constexpr usize value = 0;
};
} // namespace internal

template <usize Size, typename... Ts>
using FirstOfSize = internal::FirstOfSizeImpl<Size, Ts...>;
template <typename T, typename... Ts>
constexpr usize first_type_index = internal::FirstTypeIndex<0, T, Ts...>::value;

static_assert(first_type_index<int, char, int, float, bool, int, float> == 1);

} // namespace yacsl::meta
