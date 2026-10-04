#pragma once

namespace yacsl::meta {

template <typename T, typename U>
concept SameAs = __is_same(T, U);
template <typename T, typename... Candidates>
concept OneOf = (... || __is_same(T, Candidates));

} // namespace yacsl::meta
