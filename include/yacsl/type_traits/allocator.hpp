#pragma once

#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_traits/comparaison.hpp"
#include "yacsl/type_traits/qualifiers.hpp"

namespace yacsl::type_traits {

template <typename T>
concept AllocatorType =
    requires(T allocator, T allocator2, RemoveCVRef<T>::SizeT count,
             ranges::Slice<int, decltype(count)> slice) {
      { allocator.template allocate<int>(count) } -> SameAs<decltype(slice)>;
      { allocator.template free<int>(slice) } -> SameAs<void>;
      { allocator == allocator2 } -> SameAs<bool>;
    };

} // namespace yacsl::type_traits
