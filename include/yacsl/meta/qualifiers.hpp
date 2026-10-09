#pragma once

#include "yacsl/meta/same_as.hpp"

namespace yacsl::meta {

#ifdef __clang__
template <typename T> using RemoveReference = __remove_reference_t(T);
template <typename T> using RemoveConst     = __remove_const(T);
template <typename T> using RemoveVolatile  = __remove_volatile(T);
template <typename T> using RemoveCV        = __remove_cv(T);
template <typename T> using RemoveCVRef     = __remove_cvref(T);
#else
namespace detail {
template <typename T> struct RemoveReferenceImpl {
  using Type = __remove_reference(T);
};
template <typename T> struct RemoveConstImpl {
  using Type = T;
};
template <typename T> struct RemoveConstImpl<T const> {
  using Type = T;
};
template <typename T> struct RemoveVolatileImpl {
  using Type = T;
};
template <typename T> struct RemoveVolatileImpl<volatile T> {
  using Type = T;
};
} // namespace detail
template <typename T>
using RemoveReference                   = detail::RemoveReferenceImpl<T>::Type;
template <typename T> using RemoveConst = detail::RemoveConstImpl<T>::Type;
template <typename T>
using RemoveVolatile                    = detail::RemoveVolatileImpl<T>::Type;
template <typename T> using RemoveCV    = RemoveConst<RemoveVolatile<T>>;
template <typename T> using RemoveCVRef = RemoveCV<RemoveReference<T>>;
#endif

template <typename T> using UnderlyingEnumType = __underlying_type(T);

template <typename T>
concept ConstType = __is_const(RemoveReference<T>);
template <typename T>
concept MutableType = !__is_const(RemoveReference<T>);
template <typename T>
concept VolatileType = __is_volatile(RemoveReference<T>);
template <typename T>
concept ReferenceType = __is_reference(T);
template <typename T>
concept LValueReferenceType = SameAs<RemoveReference<T>&, T>;
template <typename T>
concept RValueReferenceType = SameAs<RemoveReference<T>&&, T>;

template <typename T, typename BaseType>
concept ReferenceTo = OneOf<T, BaseType&, BaseType&&>;

template <typename T, typename BaseType>
concept MaybeConst = OneOf<T, BaseType, BaseType const>;
template <typename T, typename BaseType>
concept MaybeVolatile = OneOf<T, BaseType, BaseType volatile>;
template <typename T, typename BaseType>
concept MaybeCV = OneOf<T, BaseType, BaseType const, BaseType volatile,
                        BaseType const volatile>;

template <typename T, typename BaseType>
concept MaybeRValueRef = OneOf<T, BaseType, BaseType&&>;
template <typename T, typename BaseType>
concept MaybeLValueRef = OneOf<T, BaseType, BaseType&>;
template <typename T, typename BaseType>
concept MaybeRef = OneOf<T, BaseType, BaseType&, BaseType&&>;

template <typename T, typename BaseType>
concept MaybeConstRef =
    OneOf<T, BaseType, BaseType&, BaseType&&, BaseType const, BaseType const&,
          BaseType const&&>;
template <typename T, typename BaseType>
concept MaybeVolatileRef =
    OneOf<T, BaseType, BaseType&, BaseType&&, BaseType volatile,
          BaseType volatile&, BaseType volatile&&>;
template <typename T, typename BaseType>
concept MaybeCVRef = SameAs<RemoveCVRef<T>, BaseType>;

namespace detail {

template <typename Src, typename Dst> struct CopyConstImpl {
  using Type = Dst;
};
template <typename Src, typename Dst> struct CopyConstImpl<Src const, Dst> {
  using Type = Dst const;
};

template <typename Src, typename Dst> struct CopyVolatileImpl {
  using Type = Dst;
};
template <typename Src, typename Dst>
struct CopyVolatileImpl<Src volatile, Dst> {
  using Type = Dst volatile;
};

template <typename Src, typename Dst> struct CopyCVImpl {
  using Type = Dst;
};
template <typename Src, typename Dst> struct CopyCVImpl<Src const, Dst> {
  using Type = Dst const;
};
template <typename Src, typename Dst> struct CopyCVImpl<Src volatile, Dst> {
  using Type = Dst volatile;
};
template <typename Src, typename Dst>
struct CopyCVImpl<Src const volatile, Dst> {
  using Type = Dst const volatile;
};

template <typename Src, typename Dst> struct CopyRValueRefImpl {
  using Type = Dst;
};
template <typename Src, typename Dst> struct CopyRValueRefImpl<Src&&, Dst> {
  using Type = Dst&&;
};

template <typename Src, typename Dst> struct CopyLValueRefImpl {
  using Type = Dst;
};
template <typename Src, typename Dst> struct CopyLValueRefImpl<Src&, Dst> {
  using Type = Dst&;
};

template <typename Src, typename Dst> struct CopyReferenceImpl {
  using Type = Dst;
};
template <typename Src, typename Dst> struct CopyReferenceImpl<Src&, Dst> {
  using Type = Dst&;
};
template <typename Src, typename Dst> struct CopyReferenceImpl<Src&&, Dst> {
  using Type = Dst&&;
};

} // namespace detail

template <typename Src, typename Dst>
using CopyConst = detail::CopyConstImpl<RemoveReference<Src>, Dst>::Type;
template <typename Src, typename Dst>
using CopyVolatile = detail::CopyVolatileImpl<RemoveReference<Src>, Dst>::Type;
template <typename Src, typename Dst>
using CopyCV = detail::CopyCVImpl<RemoveReference<Src>, Dst>::Type;

template <typename Src, typename Dst>
using CopyRValueRef = detail::CopyRValueRefImpl<Src, Dst>::Type;
template <typename Src, typename Dst>
using CopyLValueRef = detail::CopyLValueRefImpl<Src, Dst>::Type;
template <typename Src, typename Dst>
using CopyReference = detail::CopyReferenceImpl<Src, Dst>::Type;

template <typename Src, typename Dst>
using CopyCVRef = CopyReference<Src, CopyCV<Src, Dst>>;

} // namespace yacsl::meta
