#pragma once

#include "base/containers/structured_bindings.hpp"
#include "base/metaprogramming/integer_sequence.hpp"
#include "base/metaprogramming/type_manip.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

namespace internal {

template <usize Index, typename T> class TupleLeaf {
public:
	constexpr TupleLeaf() = default;

	template <typename Arg>
	// NOLINTNEXTLINE(bugprone-forwarding-reference-overload)
	explicit constexpr TupleLeaf(Arg&& other) noexcept : value_(forward<Arg>(other)) {}

	[[nodiscard]] constexpr T&			 get() noexcept { return value_; }
	[[nodiscard]] constexpr const T& get() const noexcept { return value_; }

private:
	T value_;
};

template <typename Indices, typename... Ts> class TupleImpl;
template <usize... Indices, typename... Ts>
class TupleImpl<IndexSequence<Indices...>, Ts...> : TupleLeaf<Indices, Ts>... {
	template <usize I> using Leaf = TupleLeaf<I, NthType<I, Ts...>>;

public:
	constexpr TupleImpl() = default;

	template <typename... Args>
	// NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
	constexpr TupleImpl(Args&&... args)
		requires(sizeof...(Args) == sizeof...(Ts))
			: TupleLeaf<Indices, Ts>(forward<Args>(args))... {}

	static constexpr usize size() noexcept { return sizeof...(Ts); }

	template <usize I> [[nodiscard]] constexpr NthType<I, Ts...>& get() noexcept {
		return Leaf<I>::get();
	}
	template <usize I> [[nodiscard]] constexpr const NthType<I, Ts...>& get() const noexcept {
		return Leaf<I>::get();
	}
};

} // namespace internal

template <typename... Ts> using Tuple = internal::TupleImpl<IndexSequenceToN<sizeof...(Ts)>, Ts...>;

template <typename... Ts> struct TupleSize<Tuple<Ts...>> {
	static constexpr usize value = sizeof...(Ts);
};
template <typename... Ts, usize I> struct TupleElement<base::Tuple<Ts...>, I> {
	using Type = NthType<I, Ts...>;
};

} // namespace base
