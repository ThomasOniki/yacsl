export module base.type_traits.allocator;

import base.type_traits.qualifiers;
import base.type_traits.comparaison;

namespace base {
namespace internal {
template <typename T>
concept TypeWithAllocatorMethods =
    requires(T allocator, RemoveCVRef<T>::SizeT count, int* to_free) {
      { allocator.template allocate<int>(count) } -> SameAs<int*>;
      { allocator.template free<int>(to_free, count) };
    };
} // namespace internal

export {
  template <typename T>
  concept SingletonAllocatorLike = requires() {
    { T::get() } -> internal::TypeWithAllocatorMethods;
  };
} // namespace base

namespace internal {
template <typename T> struct UnderlyingAllocatorImpl {
  using Type = T;
};
template <SingletonAllocatorLike T> struct UnderlyingAllocatorImpl<T> {
  using Type = decltype(T::get());
};
} // namespace internal

export {
  template <typename T>
  concept InstanceAllocatorLike =
      internal::TypeWithAllocatorMethods<T> && !SingletonAllocatorLike<T>;

  template <typename T>
  concept AllocatorLike = internal::TypeWithAllocatorMethods<T> || SingletonAllocatorLike<T>;

  template <typename T> using UnderlyingAllocator = internal::UnderlyingAllocatorImpl<T>::Type;
}

} // namespace base
