#pragma once

namespace base::meta {

namespace internal {
template <typename T> struct TypeIdentityImpl {
  using Type = T;
};
} // namespace internal

template <typename T> using TypeIdentity = internal::TypeIdentityImpl<T>::Type;

} // namespace base::meta
