#pragma once

namespace yacsl::type_traits {

template <typename T1, typename T2>
concept SameAs = __is_same(T1, T2);
template <typename T, typename... Candidates>
concept OneOf = (... || __is_same(T, Candidates));

} // namespace yacsl::type_traits
