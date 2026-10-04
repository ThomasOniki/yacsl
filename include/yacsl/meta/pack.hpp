#pragma once

#include "yacsl/meta/same_as.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::meta {

template <typename T, typename... Ts>
constexpr usize number_in_pack = (... + (SameAs<T, Ts>));

template <typename... Ts>
constexpr bool no_duplicates = (... && (number_in_pack<Ts, Ts...> == 1));

} // namespace yacsl::meta
