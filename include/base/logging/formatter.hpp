#pragma once

#include "base/containers/char_buffer.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/common.hpp"
#include "base/metaprogramming/declval.hpp"
#include "base/strings/str_to_int.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/type_traits/range.hpp"
#include "base/utilities/extended_types.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class FormatStringParser {
public:
	explicit constexpr FormatStringParser(const char* fmt);
	[[nodiscard]] constexpr char check_fill();
	[[nodiscard]] constexpr char check_align(char default_align);
	/// returns '\0', ' ' or '+'.
	[[nodiscard]] constexpr char check_sign();
	[[nodiscard]] constexpr u32	 check_width();
	[[nodiscard]] constexpr u32	 check_precision();

	[[nodiscard]] constexpr char check_option(char default_option, const StringView& possibilities);

	[[nodiscard]] constexpr const char* end_parse();

private:
	alignas(16) const char* fmt_ = nullptr;
};

template <typename T> class Formatter;

template <Integral T> class Formatter<T> {
public:
	constexpr const char* parse(const char* fmt) {
		FormatStringParser parser(fmt);
		fill_	 = parser.check_fill();
		align_ = parser.check_align('>');
		sign_	 = parser.check_sign();
		width_ = parser.check_width();
		type_	 = parser.check_option(Character<T> ? 'c' : 'd', "bodxc?");
		return parser.end_parse();
	}

	void format(CharBuffer& buf, T value) const;

private:
	u32	 width_ = 0;
	char fill_	= ' ';
	char align_ = '>';
	char sign_	= '\0';
	char type_	= 'd';
};

extern template void Formatter<signed char>::format(CharBuffer& buf, signed char value) const;
extern template void Formatter<wchar_t>::format(CharBuffer& buf, wchar_t value) const;
extern template void Formatter<char8_t>::format(CharBuffer& buf, char8_t value) const;
extern template void Formatter<char16_t>::format(CharBuffer& buf, char16_t value) const;
extern template void Formatter<char32_t>::format(CharBuffer& buf, char32_t value) const;

extern template void Formatter<unsigned char>::format(CharBuffer& buf, unsigned char value) const;
extern template void Formatter<unsigned short>::format(CharBuffer& buf, unsigned short value) const;
extern template void Formatter<unsigned int>::format(CharBuffer& buf, unsigned int value) const;
extern template void Formatter<unsigned long>::format(CharBuffer& buf, unsigned long value) const;
extern template void Formatter<unsigned long long>::format(CharBuffer&				buf,
																													 unsigned long long value) const;

extern template void Formatter<char>::format(CharBuffer& buf, char value) const;
extern template void Formatter<short>::format(CharBuffer& buf, short value) const;
extern template void Formatter<int>::format(CharBuffer& buf, int value) const;
extern template void Formatter<long>::format(CharBuffer& buf, long value) const;
extern template void Formatter<long long>::format(CharBuffer& buf, long long value) const;

#ifdef BASE_INT128_FOUND
extern template void Formatter<i128>::format(CharBuffer& buf, i128 value) const;
extern template void Formatter<u128>::format(CharBuffer& buf, u128 value) const;
#endif

template <FloatingPoint T> class Formatter<T> {
public:
	constexpr const char* parse(const char* fmt) {
		FormatStringParser parser(fmt);
		fill_	 = parser.check_fill();
		align_ = parser.check_align('>');
		sign_	 = parser.check_sign();
		width_ = parser.check_width();
		type_	 = parser.check_option('g', "gfea");
		return parser.end_parse();
	}

	void format(CharBuffer& buf, T value) const;

private:
	u32	 precision_ = 0;
	u32	 width_			= 0;
	char fill_			= ' ';
	char align_			= '>';
	char sign_			= '\0';
	char type_			= 'g';
};

template <> class Formatter<bool> {
public:
	constexpr const char* parse(const char* fmt) {
		FormatStringParser parser(fmt);
		fill_	 = parser.check_fill();
		align_ = parser.check_align('<');
		width_ = parser.check_width();
		type_	 = parser.check_option('s', "sdxob");
		return parser.end_parse();
	}

	void format(CharBuffer& buf, bool value) const;

private:
	u32	 width_ = 0;
	char fill_	= ' ';
	char align_ = '<';
	char type_	= 's';
};

template <> class Formatter<void*> {
public:
	constexpr const char* parse(const char* fmt) {
		FormatStringParser parser(fmt);
		fill_	 = parser.check_fill();
		align_ = parser.check_align('>');
		width_ = parser.check_width();
		return parser.end_parse();
	}

	void format(CharBuffer& buf, const void* ptr) const;

private:
	u32	 width_ = 0;
	char fill_	= ' ';
	char align_ = '<';
};

template <typename T> class Formatter<T*> : Formatter<void*> {
public:
	constexpr const char* parse(const char* fmt) { return Formatter<void*>::parse(fmt); }
	void format(CharBuffer& buf, const T* ptr) const { Formatter<void*>::format(buf, ptr); }
};

template <Character Char> class Formatter<BasicStringView<Char, usize>> {
public:
	constexpr const char* parse(const char* fmt) {
		FormatStringParser parser(fmt);
		fill_	 = parser.check_fill();
		align_ = parser.check_align('<');
		width_ = parser.check_width();
		type_	 = parser.check_option('s', "s?");
		return parser.end_parse();
	}

	using Reference = const BasicStringView<Char, usize>&;

	void format(CharBuffer& buf, Reference str) const;

private:
	u32	 width_ = 0;
	char fill_	= ' ';
	char align_ = '<';
	char type_	= 's';
};

extern template void Formatter<BasicStringView<char>>::format(CharBuffer&, Reference) const;
extern template void Formatter<BasicStringView<unsigned char>>::format(CharBuffer&,
																																			 Reference) const;
extern template void Formatter<BasicStringView<signed char>>::format(CharBuffer&, Reference) const;
extern template void Formatter<BasicStringView<wchar_t>>::format(CharBuffer&, Reference) const;
extern template void Formatter<BasicStringView<char8_t>>::format(CharBuffer&, Reference) const;
extern template void Formatter<BasicStringView<char16_t>>::format(CharBuffer&, Reference) const;
extern template void Formatter<BasicStringView<char32_t>>::format(CharBuffer&, Reference) const;

template <Character Char, typename SizeT>
class Formatter<BasicStringView<Char, SizeT>> : Formatter<BasicStringView<Char, usize>> {
public:
	constexpr const char* parse(const char* fmt) {
		return Formatter<BasicStringView<Char>>::parse(fmt);
	}
	void format(CharBuffer& buf, const BasicStringView<Char, SizeT>& str) const {
		return Formatter<BasicStringView<Char>>::format(
				buf, BasicStringView<Char>(str.data(), static_cast<usize>(str.size())));
	}
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
template <Character Char, usize N> class Formatter<Char[N]> : Formatter<BasicStringView<Char>> {
public:
	constexpr const char* parse(const char* fmt) {
		return Formatter<BasicStringView<Char>>::parse(fmt);
	}
	// NOLINTNEXTLINE(modernize-avoid-c-arrays)
	void format(CharBuffer& buf, const Char (&str)[N]) const {
		return Formatter<BasicStringView<Char>>::format(buf, BasicStringView(str, N - 1));
	}
};
template <Character Char, usize N>
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
class Formatter<const Char[N]> : Formatter<BasicStringView<Char>> {
public:
	constexpr const char* parse(const char* fmt) {
		return Formatter<BasicStringView<Char>>::parse(fmt);
	}
	// NOLINTNEXTLINE(modernize-avoid-c-arrays)
	void format(CharBuffer& buf, const Char (&str)[N]) const {
		return Formatter<BasicStringView<Char>>::format(buf, BasicStringView(str, N - 1));
	}
};

template <Range T> class Formatter<T> : Formatter<RemoveCVRef<decltype(*declval<T>().begin())>> {
public:
	constexpr const char* parse(const char* fmt) { return Base::parse(fmt); }

	void format(CharBuffer& buf, const T& range) {
		buf.append('[');
		auto iter = range.begin();
		auto end	= range.end();
		if (iter != end) {
			Base::format(buf, *iter);
			++iter;
			for (; iter != end; ++iter) {
				buf.append(", ");
				Base::format(buf, *iter);
			}
		}
		buf.append(']');
	}

private:
	using Element = RemoveCVRef<decltype(*declval<T>().begin())>;
	using Base		= Formatter<Element>;
};

constexpr FormatStringParser::FormatStringParser(const char* fmt) : fmt_(fmt) {
	if (*fmt_ != '{') {
		log_error("FormatStringParser called on a string that doesn't start with '{'.");
	}

	++fmt_;

	if (*fmt_ == '\0') {
		log_error("'{' without '}' in format string.");
	}
}

constexpr char FormatStringParser::check_fill() {
	if (*fmt_ == '0' || fmt_[1] == '>' || fmt_[1] == '<' || fmt_[1] == '^') {
		return *fmt_++;
	}
	return ' ';
}

constexpr char FormatStringParser::check_align(char default_align) {
	return check_option(default_align, "<>^");
}

constexpr char FormatStringParser::check_sign() {
	const char sign = check_option('-', "+- ");
	return sign == '-' ? '\0' : sign;
}

constexpr u32 FormatStringParser::check_width() {
	const char* begin = fmt_;
	fmt_							= num_end(fmt_);
	return str_to_int<u32>({begin, fmt_});
}

constexpr u32 FormatStringParser::check_precision() {
	if (*fmt_ == '.') {
		++fmt_;
		return check_width();
	}
	return 0;
}

constexpr char FormatStringParser::check_option(char							default_option,
																								const StringView& possibilities) {
	for (const char possibility : possibilities) {
		if (possibility == *fmt_) {
			++fmt_;
			return possibility;
		}
	}
	return default_option;
}

constexpr const char* FormatStringParser::end_parse() {
	if (*fmt_ == '\0') {
		log_error("Expected '}' before '\0' in format string.");
	}
	if (*fmt_ != '}') {
		log_error("Invalid format string.");
	}

	++fmt_;
	return fmt_;
}

} // namespace base
