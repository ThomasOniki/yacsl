export module base.metaprogramming.integer_sequence;

import base.utilities.type_aliases;

namespace base {

export template <typename T, T... Ints> class IntegerSequence {
  using Type = T;
};

namespace internal {
template <typename A, typename B> struct MergeIntegerSequencesImpl;
template <typename T, T... Ints1, T... Ints2>
struct MergeIntegerSequencesImpl<IntegerSequence<T, Ints1...>, IntegerSequence<T, Ints2...>> {
  using Type = IntegerSequence<T, Ints1..., Ints2...>;
};
template <typename T, T Int, typename Seq> struct PrependToIntegerSequenceImpl;
template <typename T, T Int, T... Ints>
struct PrependToIntegerSequenceImpl<T, Int, IntegerSequence<T, Ints...>> {
  using Type = IntegerSequence<T, Int, Ints...>;
};
template <typename T, T Int, typename Seq> struct AppendToIntegerSequenceImpl;
template <typename T, T Int, T... Ints>
struct AppendToIntegerSequenceImpl<T, Int, IntegerSequence<T, Ints...>> {
  using Type = IntegerSequence<T, Ints..., Int>;
};
} // namespace internal

export {

  template <usize... Indices> using IndexSequence = IntegerSequence<usize, Indices...>;

#ifdef __clang__
  template <typename T, T N> using IntegerSequenceToN = __make_integer_seq<IntegerSequence, T, N>;
  template <usize N> using IndexSequenceToN = __make_integer_seq<IntegerSequence, usize, N>;
#else
  template <typename T, T N> using IntegerSequenceToN = IntegerSequence<T, __integer_pack(N)...>;
  template <usize N> using IndexSequenceToN = IntegerSequence<usize, __integer_pack(N)...>;
#endif

  template <typename A, typename B>
  using MergeIntegerSequences = internal::MergeIntegerSequencesImpl<A, B>::Type;

  template <typename Seq, Seq::Type Elem>
  using PrependToIntegerSequence =
      internal::PrependToIntegerSequenceImpl<typename Seq::Type, Elem, Seq>;
  template <typename Seq, Seq::Type Elem>
  using AppendToIntegerSequence =
      internal::AppendToIntegerSequenceImpl<typename Seq::Type, Elem, Seq>;
}
} // namespace base
