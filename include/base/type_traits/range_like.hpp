#pragma once

#include "base/meta/declval.hpp"
#include "base/type_traits/comparaison.hpp"
#include "base/type_traits/convertible_to.hpp"

namespace base::type_traits {

template <typename T>
concept RangeLike = requires(T range) {
  { range.begin() } -> SameAs<decltype(range.end())>;
  { *range.begin() };
  { ++meta::declval<decltype(range.begin())>() };
};

template <typename Range, typename T>
concept RangeOf = RangeLike<Range> && requires(Range range) {
  { *range.begin() } -> ConvertibleTo<T>;
};

} // namespace base::type_traits
