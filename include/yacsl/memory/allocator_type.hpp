#pragma once

#include "yacsl/memory/alignment.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/same_as.hpp"

namespace yacsl::memory {

template <typename A>
concept AllocatorType = requires(A allocator, A::SizeT size, Alignment align,
                                 ranges::Slice<u8, typename A::SizeT> slice) {
  { allocator.allocate(size, align) } -> meta::SameAs<decltype(slice)>;
  { allocator.deallocate(slice) } -> meta::SameAs<void>;
  {
    allocator.reallocate_nocopy(slice, size, align)
  } -> meta::SameAs<decltype(slice)>;
  { allocator == allocator } -> meta::SameAs<bool>;

  A();
  A(allocator);
  A(memory::move(allocator));
  allocator = allocator;
  allocator = memory::move(allocator);
};

} // namespace yacsl::memory
