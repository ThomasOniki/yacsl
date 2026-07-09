export module base.type_traits.range_like;

import base.type_traits.comparaison;
import base.type_traits.convertible_to;
import base.metaprogramming.declval;

export namespace base {

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
