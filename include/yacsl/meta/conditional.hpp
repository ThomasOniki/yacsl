#pragma once

namespace yacsl::meta {

namespace detail {
template <bool Cond, typename If, typename Else> struct ConditionalTypeImpl {
  using Type = If;
};
template <typename If, typename Else>
struct ConditionalTypeImpl<false, If, Else> {
  using Type = Else;
};
} // namespace detail

template <bool Cond, typename If, typename Else>
using Conditional = detail::ConditionalTypeImpl<Cond, If, Else>::Type;

} // namespace yacsl::meta
