#pragma once

#include "base/metaprogramming/declval.hpp"
#include "base/type_traits/comparaison.hpp"
#include "base/type_traits/convertible_to.hpp"

namespace base {

template <typename T>
concept RangeLike = requires(T range) {
	{ range.begin() } -> SameAs<decltype(range.end())>;
	{ *range.begin() };
	{ ++declval<decltype(range.begin())>() };
};

template <typename Range, typename T>
concept RangeOf = RangeLike<Range> && requires(Range range) {
	{ *range.begin() } -> ConvertibleTo<T>;
};

} // namespace base
