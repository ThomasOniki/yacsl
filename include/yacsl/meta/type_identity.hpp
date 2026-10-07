#pragma once

namespace yacsl::meta {

namespace detail {
template <typename T> struct TypeIdentityImpl {
  using Type = T;
};
} // namespace detail

template <typename T> using TypeIdentity = detail::TypeIdentityImpl<T>::Type;

} // namespace yacsl::meta
