#pragma once

#include "base/metaprogramming/declval.hpp"
#include "base/type_traits/comparaison.hpp"

namespace base {

template <typename T>
concept RangeLike = requires(T range) {
	{ range.begin() } -> SameAs<decltype(range.end())>;
	{ *range.begin() };
	{ ++declval<decltype(range.begin())>() };
};

} // namespace base
