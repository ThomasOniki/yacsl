#pragma once

#include "base/type_traits/allocator.hpp"

namespace base {

template <typename Allocator> class MaybeAllocatorMember {
protected:
	Allocator& allocator() noexcept { return allocator_; }

private:
	Allocator allocator_;
};

template <SingletonAllocatorLike Allocator> class MaybeAllocatorMember<Allocator> {
protected:
	UnderlyingAllocator<Allocator>& allocator() noexcept { return Allocator::get(); }
};

} // namespace base
