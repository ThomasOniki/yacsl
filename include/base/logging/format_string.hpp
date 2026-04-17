#pragma once

#include "base/containers/char_buffer.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/containers/tuple.hpp"
#include "base/logging/common.hpp"
#include "base/logging/formatter.hpp"
#include "base/metaprogramming/integer_sequence.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/fail_consteval.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

namespace internal {

template <typename FString> class FormatToImpl;

template <usize... Indices, typename... Args>
class FormatStringImpl<IndexSequence<Indices...>, Args...> {
public:
	// NOLINTNEXTLINE(google-explicit-constructor)
	consteval FormatStringImpl(const char* fmt) {
		(..., [&]() {
			const char* segment_begin = fmt;
			for (; *fmt != '{'; ++fmt) {
				if (*fmt == '\0') {
					fail_consteval("Too few format specifiers for the argument count.");
				}
			}
			segments_[Indices] = {segment_begin, fmt};
			fmt								 = formatters_.template get<Indices>().parse(fmt);
		}());

		const char* segment_begin = fmt;
		for (; *fmt != '\0'; ++fmt) {
			if (*fmt == '{') {
				fail_consteval("Too many format specifiers for the arguments count.");
			}
		}
		segments_[sizeof...(Args)] = {segment_begin, fmt};
	}

	friend class FormatToImpl<FormatStringImpl>;

public:
	using TypeIdentity = FormatStringImpl;

private:
	mutable Tuple<Formatter<RemoveCVRef<Args>>...> formatters_;

	StackArray<StringView, sizeof...(Args) + 1> segments_;
};

template <> class FormatStringImpl<IndexSequence<>> {
public:
	// NOLINTNEXTLINE
	template <usize N> consteval FormatStringImpl(const char (&fmt)[N]) : segments_{fmt} {}

	friend class FormatToImpl<FormatStringImpl>;

public:
	using TypeIdentity = FormatStringImpl;

private:
	StackArray<StringView, 1> segments_;
};

template <usize... Indices, typename... Args>
class FormatToImpl<FormatStringImpl<IndexSequence<Indices...>, Args...>> {
public:
	static void format_to_impl(CharBuffer&																								 buf,
														 const FormatStringImpl<IndexSequence<Indices...>, Args...>& fmt,
														 Args&&... args) {
		(..., [&]() {
			buf.append(fmt.segments_[Indices]);
			fmt.formatters_.template get<Indices>().format(buf, base::forward<Args>(args));
		}());
		buf.append(fmt.segments_[sizeof...(Args)]);
	}
};

} // namespace internal

template <typename... Args>
void format_to(CharBuffer& buf, const FormatString<Args...>& fmt, Args&&... args) {
	internal::FormatToImpl<FormatString<Args...>>::format_to_impl(buf, fmt,
																																base::forward<Args>(args)...);
}

} // namespace base
