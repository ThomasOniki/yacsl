#pragma once

#include "base/containers/stack_array.hpp"
#include "base/containers/stack_array_storage.hpp"
#include "base/metaprogramming/type_manip.hpp"
#include "base/type_traits/allocator.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename Allocator> class AllocatorMember {
protected:
  Allocator& allocator() noexcept BASE_LIFETIMEBOUND { return allocator_; }

private:
  Allocator allocator_;
};
template <SingletonAllocatorLike Allocator> class AllocatorMember<Allocator> {
protected:
  UnderlyingAllocator<Allocator>& allocator() noexcept { return Allocator::get(); }
};

template <typename T, usize N, Integral SizeT> class FixedBufferMember {
protected:
  static constexpr bool trivial = TriviallyCopyable<T> && TriviallyDestructible<T>;
  ConditionalT<trivial, StackArray<T, N, SizeT>, StackArrayStorage<T, N, SizeT>> fixed_buffer_{};
};
template <typename T, Integral SizeT> class FixedBufferMember<T, 0, SizeT> {
protected:
  struct DummyFixedBuffer {
    [[nodiscard]] T*    data() const noexcept { return nullptr; }
    [[nodiscard]] SizeT size() const noexcept { return 0; }
  };
  static constexpr DummyFixedBuffer fixed_buffer_{};
};

} // namespace base
