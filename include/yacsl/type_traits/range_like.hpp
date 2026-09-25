#pragma once

#include "yacsl/meta/declval.hpp"
#include "yacsl/type_traits/comparaison.hpp"
#include "yacsl/type_traits/convertible_to.hpp"

namespace yacsl::type_traits {

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

} // namespace yacsl::type_traits
