#pragma once

#include "yacsl/type_aliases.hpp"

namespace yacsl::meta {

template <typename T, T... Ints> struct IntegerSequence {
  using Type = T;

  template <usize I> static constexpr T at_index = Ints...[I];
};

namespace detail {
template <typename First, typename... Others> struct MergeIntegerSequencesImpl {
  using Type = MergeIntegerSequencesImpl<
      First, typename MergeIntegerSequencesImpl<Others...>::Type>::Type;
};
template <typename T, T... Ints1, T... Ints2>
struct MergeIntegerSequencesImpl<IntegerSequence<T, Ints1...>,
                                 IntegerSequence<T, Ints2...>> {
  using Type = IntegerSequence<T, Ints1..., Ints2...>;
};
template <typename T> struct MergeIntegerSequencesImpl<T> {
  using Type = T;
};
} // namespace detail

template <usize... Indices>
using IndexSequence = IntegerSequence<usize, Indices...>;

#if __has_builtin(__make_integer_seq)
template <typename T, T N>
using IntegerSequenceToN = __make_integer_seq<IntegerSequence, T, N>;
template <usize N>
using IndexSequenceToN = __make_integer_seq<IntegerSequence, usize, N>;
#else
template <typename T, T N>
using IntegerSequenceToN = IntegerSequence<T, __integer_pack(N)...>;
template <usize N>
using IndexSequenceToN = IntegerSequence<usize, __integer_pack(N)...>;
#endif

template <typename... Ts>
using MergeIntegerSequences = detail::MergeIntegerSequencesImpl<Ts...>::Type;

template <typename Seq, Seq::Type... Ints>
using PrependToIntegerSequence =
    MergeIntegerSequences<IntegerSequence<typename Seq::Type, Ints...>, Seq>;
template <typename Seq, Seq::Type... Ints>
using AppendToIntegerSequence =
    MergeIntegerSequences<Seq, IntegerSequence<typename Seq::Type, Ints...>>;

} // namespace yacsl::meta
