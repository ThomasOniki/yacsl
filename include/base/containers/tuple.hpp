#pragma once

#include "base/attributes.hpp"
#include "base/containers/structured_bindings.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/metaprogramming/integer_sequence.hpp"
#include "base/metaprogramming/type_manip.hpp"
#include "base/type_traits/comparaison.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base::internal {

template <usize Index, typename T> class TupleLeaf {
public:
  constexpr TupleLeaf() = default;

  template <typename Arg>
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
  explicit constexpr TupleLeaf(Arg&& other) noexcept
    requires(!SameAs<T, T &&>)
      : value_(base::forward<Arg>(other)) {}

  template <typename Arg>
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
  explicit constexpr TupleLeaf(BASE_LIFETIMEBOUND Arg&& other) noexcept
    requires SameAs<T, T&&>
      : value_(base::forward<Arg>(other)) {}

  [[nodiscard]] constexpr T&
  get(this TupleLeaf& self BASE_LIFETIMEBOUND) noexcept {
    return self.value_;
  }
  [[nodiscard]] constexpr T&&
  get(this TupleLeaf&& self BASE_LIFETIMEBOUND) noexcept {
    return base::move(self).value_;
  }
  [[nodiscard]] constexpr T const&
  get(this TupleLeaf const& self BASE_LIFETIMEBOUND) noexcept {
    return self.value_;
  }
  [[nodiscard]] constexpr T const&&
  get(this TupleLeaf const&& self BASE_LIFETIMEBOUND) noexcept {
    return base::move(self).value_;
  }

private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
  T value_;
};

template <typename Indices, typename... Ts> class TupleImpl;
template <usize... Indices, typename... Ts>
class TupleImpl<IndexSequence<Indices...>, Ts...> : TupleLeaf<Indices, Ts>... {
  template <usize I> using Leaf = TupleLeaf<I, NthType<I, Ts...>>;

public:
  constexpr TupleImpl() noexcept : Leaf<Indices>()... {}

  template <typename... Args>
  constexpr TupleImpl(Args&&... args)
    requires(sizeof...(Args) == sizeof...(Ts))
      : TupleLeaf<Indices, Ts>(base::forward<Args>(args))... {}

  constexpr usize size(this TupleImpl const& /*self*/) noexcept {
    return sizeof...(Ts);
  }

  template <usize I, typename Self>
  [[nodiscard]] constexpr auto&&
  get(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).Leaf<I>::get();
  }
};

} // namespace base::internal

namespace base {

template <typename... Ts>
using Tuple = internal::TupleImpl<IndexSequenceToN<sizeof...(Ts)>, Ts...>;

template <typename... Ts> struct TupleSize<Tuple<Ts...>> {
  static constexpr usize value = sizeof...(Ts);
};
template <typename... Ts, usize I> struct TupleElement<base::Tuple<Ts...>, I> {
  using Type = NthType<I, Ts...>;
};

} // namespace base
