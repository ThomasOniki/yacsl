#pragma once

#include "yacsl/meta/integer_sequence.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::meta {

template <typename... Ts> struct TypeSequence {};

namespace internal {
template <typename... Ts> struct MergeTypeSequencesImpl {
  using Type = TypeSequence<>;
};
template <typename First, typename... Ts>
struct MergeTypeSequencesImpl<First, Ts...> {
  using Type = MergeTypeSequencesImpl<
      First, typename MergeTypeSequencesImpl<Ts...>::Type>::Type;
};
template <typename... Ts1, typename... Ts2>
struct MergeTypeSequencesImpl<TypeSequence<Ts1...>, TypeSequence<Ts2>...> {
  using Type = TypeSequence<Ts1..., Ts2...>;
};
template <typename T> struct MergeTypeSequencesImpl<T> {
  using Type = T;
};
template <typename T, decltype(auto) Dependency> using DependantTypedef = T;

template <typename T, typename Indices> struct RepeatTypeSequenceImpl;
template <typename T, usize... Indices>
struct RepeatTypeSequenceImpl<T, IndexSequence<Indices...>> {
  using Type = MergeTypeSequencesImpl<DependantTypedef<T, Indices>...>;
};

template <typename T, typename Indices> struct RepeatTypeImpl;
template <typename T, usize... Indices>
struct RepeatTypeImpl<T, IndexSequence<Indices...>> {
  using Type = TypeSequence<DependantTypedef<T, Indices>...>;
};

} // namespace internal

template <typename... Ts>
using MergeTypeSequences = internal::MergeTypeSequencesImpl<Ts...>::Type;
template <typename T, typename... Ts>
using PrependToTypeSequence = MergeTypeSequences<TypeSequence<Ts...>, T>;
template <typename T, typename... Ts>
using AppendToTypeSequence = MergeTypeSequences<T, TypeSequence<Ts...>>;

template <typename T, usize N>
using RepeatTypeSequence =
    internal::RepeatTypeSequenceImpl<T, IndexSequenceToN<N>>;
template <typename T, usize N>
using RepeatType = internal::RepeatTypeImpl<T, IndexSequenceToN<N>>;

} // namespace yacsl::meta
