#pragma once

#include "base/attributes.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/stack_array_storage.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/metaprogramming/type_manip.hpp"
#include "base/type_traits/allocator.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <AllocatorLike Allocator> class AllocatorMember {
protected:
  Allocator& allocator(this AllocatorMember& self BASE_LIFETIMEBOUND) noexcept {
    return self.allocator_;
  }
  Allocator&& allocator(this AllocatorMember&& self BASE_LIFETIMEBOUND) noexcept {
    return base::move(self).allocator_;
  }
  Allocator const& allocator(this AllocatorMember const& self BASE_LIFETIMEBOUND) noexcept {
    return self.allocator_;
  }
  Allocator const&& allocator(this AllocatorMember const&& self BASE_LIFETIMEBOUND) noexcept {
    return base::move(self).allocator_;
  }

private:
  Allocator allocator_;
};
template <SingletonAllocatorLike Allocator> class AllocatorMember<Allocator> {
protected:
  auto& allocator(this const AllocatorMember& /*self*/) noexcept { return Allocator::get(); }
};

template <typename T, usize N, Integral SizeT> class FixedBufferMember {
protected:
  static constexpr bool trivial = TriviallyCopyable<T> && TriviallyDestructible<T>;
  ConditionalT<trivial, StackArray<T, N, SizeT>, StackArrayStorage<T, N, SizeT>> fixed_buffer_{};
};
template <typename T, Integral SizeT> class FixedBufferMember<T, 0, SizeT> {
protected:
  struct DummyFixedBuffer {
    [[nodiscard]] T*    data(this DummyFixedBuffer const& /*self*/) noexcept { return nullptr; }
    [[nodiscard]] T*    begin(this DummyFixedBuffer const& /*self*/) noexcept { return nullptr; }
    [[nodiscard]] T*    end(this DummyFixedBuffer const& /*self*/) noexcept { return nullptr; }
    [[nodiscard]] SizeT size(this DummyFixedBuffer const& /*self*/) noexcept { return 0; }
  };
  static constexpr DummyFixedBuffer fixed_buffer_{};
};

} // namespace base
