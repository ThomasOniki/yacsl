#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/containers/structured_bindings.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/integer_sequence.hpp"
#include "yacsl/meta/same_as.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::containers {
namespace detail {

template <usize Index, typename T> class TupleLeaf {
public:
  constexpr TupleLeaf() = default;

  template <typename Arg>
  // NOLINTNEXTLINE(bugprone-memory::forwarding-reference-overload)
  explicit constexpr TupleLeaf(Arg&& other) noexcept
    requires(!meta::SameAs<T, T&&>)
      : value_(memory::forward<Arg>(other)) {}

  template <typename Arg>
  // NOLINTNEXTLINE(bugprone-memory::forwarding-reference-overload)
  explicit constexpr TupleLeaf(YACSL_LIFETIMEBOUND Arg&& other) noexcept
    requires meta::SameAs<T, T&&>
      : value_(memory::forward<Arg>(other)) {}

  [[nodiscard]] constexpr T&
  get(this TupleLeaf& self YACSL_LIFETIMEBOUND) noexcept {
    return self.value_;
  }
  [[nodiscard]] constexpr T&&
  get(this TupleLeaf&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::move(self).value_;
  }
  [[nodiscard]] constexpr T const&
  get(this TupleLeaf const& self YACSL_LIFETIMEBOUND) noexcept {
    return self.value_;
  }
  [[nodiscard]] constexpr T const&&
  get(this TupleLeaf const&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::move(self).value_;
  }

private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
  T value_;
};

template <typename Indices, typename... Ts> class TupleImpl;
template <usize... Indices, typename... Ts>
class TupleImpl<meta::IndexSequence<Indices...>, Ts...>
    : TupleLeaf<Indices, Ts>... {
  template <usize I> using Leaf = TupleLeaf<I, Ts...[I]>;

public:
  constexpr TupleImpl() noexcept : Leaf<Indices>()... {}

  template <typename... Args>
  constexpr TupleImpl(Args&&... args)
    requires(sizeof...(Args) == sizeof...(Ts))
      : TupleLeaf<Indices, Ts>(memory::forward<Args>(args))... {}

  constexpr usize len(this TupleImpl const& /*self*/) noexcept {
    return sizeof...(Ts);
  }

  template <usize I, typename Self>
  [[nodiscard]] constexpr auto&&
  get(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).Leaf<I>::get();
  }
};

} // namespace detail

template <typename... Ts>
using Tuple = detail::TupleImpl<meta::IndexSequenceToN<sizeof...(Ts)>, Ts...>;

template <typename... Ts> struct TupleSize<Tuple<Ts...>> {
  static constexpr usize value = sizeof...(Ts);
};
template <typename... Ts, usize I> struct TupleElement<Tuple<Ts...>, I> {
  using Type = Ts...[I];
};

} // namespace yacsl::containers
