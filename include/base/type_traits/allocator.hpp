#pragma once

#include "base/ranges/slice.hpp"
#include "base/type_traits/comparaison.hpp"
#include "base/type_traits/qualifiers.hpp"

namespace base::type_traits {

template <typename T>
concept AllocatorType =
    requires(T allocator, T allocator2, RemoveCVRef<T>::SizeT count,
             ranges::Slice<int, decltype(count)> slice) {
      { allocator.template allocate<int>(count) } -> SameAs<decltype(slice)>;
      { allocator.template free<int>(slice) } -> SameAs<void>;
      { allocator == allocator2 } -> SameAs<bool>;
    };

} // namespace base::type_traits
