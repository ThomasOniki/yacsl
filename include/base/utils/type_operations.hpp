#pragma once

namespace base {

namespace detail {
template <bool Cond, typename If, typename Else> struct ConditionalT {
	using Type = If;
};
template <typename If, typename Else> struct ConditionalT<false, If, Else> {
	using Type = Else;
};
} // namespace detail

template <bool Cond, typename If, typename Else> using ConditionalT = detail::ConditionalT<Cond, If, Else>::Type;

#if __has_builtin(__is_same)
template <typename T1, typename T2>
concept SameAs = __is_same(T1, T2);
#else
#error No __is_same builtin.
#endif

} // namespace base
