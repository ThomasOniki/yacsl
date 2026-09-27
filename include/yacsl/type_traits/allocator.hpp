#pragma once

#include "yacsl/containers/option.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_traits/comparaison.hpp"

namespace yacsl::type_traits {

template <typename A>
concept AllocatorType =
    requires(A allocator, A::SizeT size, memory::Alignment align,
             ranges::Slice<uchar, typename A::SizeT> slice) {
      { allocator.allocate(size, align) } -> SameAs<decltype(slice)>;
      { allocator.deallocate(slice) } -> SameAs<void>;
      {
        allocator.resize_in_place(slice, size)
      } -> SameAs<containers::Option<decltype(slice)>>;
      { allocator == allocator } -> SameAs<bool>;

      A();
      A(allocator);
      A(memory::move(allocator));
      allocator = allocator;
      allocator = memory::move(allocator);
    };

} // namespace yacsl::type_traits
