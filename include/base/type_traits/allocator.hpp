#pragma once

#include "base/ranges/slice.hpp"
#include "base/type_traits/comparaison.hpp"
#include "base/type_traits/qualifiers.hpp"

namespace base {
namespace internal {
template <typename T>
concept TypeWithAllocatorMethods =
    requires(T allocator, RemoveCVRef<T>::SizeT count,
             Slice<int, decltype(count)> slice) {
      { allocator.template allocate<int>(count) } -> SameAs<decltype(slice)>;
      { allocator.template free<int>(slice) };
    };
} // namespace internal

template <typename T>
concept SingletonAllocatorLike = requires() {
  { T::get() } -> internal::TypeWithAllocatorMethods;
};

namespace internal {
template <typename T> struct UnderlyingAllocatorImpl {
  using Type = T;
};
template <SingletonAllocatorLike T> struct UnderlyingAllocatorImpl<T> {
  using Type = decltype(T::get());
};
} // namespace internal

template <typename T>
concept InstanceAllocatorLike =
    internal::TypeWithAllocatorMethods<T> && !SingletonAllocatorLike<T>;

template <typename T>
concept AllocatorLike =
    internal::TypeWithAllocatorMethods<T> || SingletonAllocatorLike<T>;

template <typename T>
using UnderlyingAllocator = internal::UnderlyingAllocatorImpl<T>::Type;

} // namespace base
