export module base.metaprogramming.type_identity;

namespace base {

namespace internal {
template <typename T> struct TypeIdentityImpl {
  using Type = T;
};
} // namespace internal

template <typename T> using TypeIdentity = internal::TypeIdentityImpl<T>::Type;

} // namespace base
