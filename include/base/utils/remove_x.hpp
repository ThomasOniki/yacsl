#pragma once

namespace base {

#if __has_builtin(__remove_reference_t)
template <typename T> using RemoveReference = __remove_reference_t(T);
#else
#error No __remove_reference_t builtin.
#endif

#if __has_builtin(__remove_const)
template <typename T> using RemoveConst = __remove_const(T);
#else
#error No __remove_const builtin.
#endif

#if __has_builtin(__remove_volatile)
template <typename T> using RemoveVolatile = __remove_volatile(T);
#else
#error No __remove_volatile builtin.
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

#if __has_builtin(__remove_pointer)
template <typename T> using RemovePointer = __remove_pointer(T);
#else
#error No __remove_pointer builtin.
#endif

} // namespace base
