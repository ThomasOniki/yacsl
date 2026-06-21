export module base.type_traits.qualifiers;

export namespace base {
template <typename T> using UnderlyingEnumType = __underlying_type(T);
} // namespace base

#ifdef __clang__
export namespace base {
template <typename T> using RemoveReference = __remove_reference_t(T);
template <typename T> using RemoveConst     = __remove_const(T);
template <typename T> using RemoveVolatile  = __remove_volatile(T);
template <typename T> using RemoveCV        = __remove_cv(T);
template <typename T> using RemoveCVRef     = __remove_cvref(T);
} // namespace base
#else
namespace base::internal {
template <typename T> struct RemoveReferenceImpl {
  using Type = __remove_reference(T);
};
template <typename T> struct RemoveConstImpl {
  using Type = T;
};
template <typename T> struct RemoveConstImpl<const T> {
  using Type = T;
};
template <typename T> struct RemoveVolatileImpl {
  using Type = T;
};
template <typename T> struct RemoveVolatileImpl<volatile T> {
  using Type = T;
};
} // namespace base::internal

export namespace base {
template <typename T> using RemoveReference = internal::RemoveReferenceImpl<T>::Type;
template <typename T> using RemoveConst     = internal::RemoveConstImpl<T>::Type;
template <typename T> using RemoveVolatile  = internal::RemoveVolatileImpl<T>::Type;
template <typename T> using RemoveCV        = RemoveConst<RemoveVolatile<T>>;
template <typename T> using RemoveCVRef     = RemoveCV<RemoveReference<T>>;
} // namespace base
#endif
