module;

#include "base/attributes.hpp"

export module base.containers.base_classes;

import base.containers.stack_array;
import base.containers.stack_array_storage;
import base.type_traits.categories;
import base.type_traits.trivial;
import base.type_traits.allocator;
import base.utilities.type_aliases;
import base.metaprogramming.type_manip;

export namespace base {

template <AllocatorLike Allocator> class AllocatorMember {
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
    [[nodiscard]] T*    begin() const noexcept { return nullptr; }
    [[nodiscard]] T*    end() const noexcept { return nullptr; }
    [[nodiscard]] SizeT size() const noexcept { return 0; }
  };
  static constexpr DummyFixedBuffer fixed_buffer_{};
};

} // namespace base
