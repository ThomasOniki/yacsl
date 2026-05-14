#pragma once

namespace base {

#ifdef BASE_COMPILER_GCC
namespace internal {
template <typename T> struct RemoveReferenceImpl {
  using Type = __remove_reference(T);
};
} // namespace internal
template <typename T> using RemoveReference = internal::RemoveReferenceImpl<T>::Type;
#else
template <typename T> using RemoveReference = __remove_reference_t(T);
#endif

#if __has_builtin(__remove_const)
template <typename T> using RemoveConst = __remove_const(T);
#else
namespace internal {
template <typename T> struct RemoveConstImpl {
  using Type = T;
};
template <typename T> struct RemoveConstImpl<const T> {
  using Type = T;
};
} // namespace internal
template <typename T> using RemoveConst = internal::RemoveConstImpl<T>;
#endif

#if __has_builtin(__remove_volatile)
template <typename T> using RemoveVolatile = __remove_volatile(T);
#else
namespace internal {
template <typename T> struct RemoveVolatileImpl {
  using Type = T;
};
template <typename T> struct RemoveVolatileImpl<volatile T> {
  using Type = T;
};
} // namespace internal
template <typename T> using RemoveVolatile = internal::RemoveVolatileImpl<T>;
#endif

#if __has_builtin(__remove_cv)
template <typename T> using RemoveCV = __remove_cv(T);
#else
template <typename T> using RemoveCV = RemoveConst<RemoveVolatile<T>>;
#endif

#if __has_builtin(__remove_cvref)
template <typename T> using RemoveCVRef = __remove_cvref(T);
#else
template <typename T> using RemoveCVRef = RemoveCV<RemoveReference<T>>;
#endif

} // namespace base
