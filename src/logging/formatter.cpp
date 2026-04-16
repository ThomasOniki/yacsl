#include "base/logging/formatter.hpp"
#include "base/containers/char_buffer.hpp"
#include "base/containers/fwds.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/strings/itoa.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/limits.hpp"
#include "base/type_traits/sign.hpp"
#include "base/utilities/extended_types.hpp"
#include "base/utilities/type_aliases.hpp"
#include <string.h>

namespace base {

namespace {
struct FormatOptions {
	alignas(8) u32 width;
	char fill;
	char align;
};
} // namespace

void static format_string_view(CharBuffer& buf, const StringView& str, const FormatOptions& opts) {
	if (opts.width <= str.size()) {
		buf.append(str);
		return;
	}
	char* begin = buf.grow(opts.width);
	if (opts.align == '<') {
		memcpy(begin, str.data(), str.size());
		memset(begin + str.size(), opts.fill, opts.width - str.size());
	} else if (opts.align == '>') {
		const u32 pad = opts.width - static_cast<u32>(str.size());
		memset(begin, opts.fill, pad);
		memcpy(begin + pad, str.data(), str.size());
	} else {
		const u32 pad = (opts.width - static_cast<u32>(str.size())) / 2;
		memset(begin, opts.fill, opts.width);
		memcpy(begin + pad, str.data(), str.size());
	}
}

void Formatter<StringView>::format(CharBuffer& buf, const StringView& str) const {
	format_string_view(buf, str, FormatOptions{.width = width_, .fill = fill_, .align = align_});
}

void Formatter<const char*>::format(CharBuffer& buf, const char* str) {
	Formatter<StringView>::format(buf, {str, strlen(str)});
}
void Formatter<char*>::format(CharBuffer& buf, const char* str) {
	Formatter<StringView>::format(buf, {str, strlen(str)});
}

template <Integral T, u32 Base, u32 Offset> static constexpr StackArray<T, 128> gen_powers() {
	StackArray<T, 128> ret = {};
	T									 pow = 1;
	for (u32 i = Offset; i < 128; ++i) {
		if (pow <= Limits<T>::max / Base) {
			pow *= Base;
		} else {
			pow = Limits<T>::max;
		}
		ret[i] = pow;
	}
	return ret;
}

template <typename T, u32 Base> static char* itoa_from_end_base_x(char* buf_end, T value) {
	if constexpr (Base == 2) {
		return itoa_from_end_bin(buf_end, value);
	}
	if constexpr (Base == 8) {
		return itoa_from_end_oct(buf_end, value);
	}
	if constexpr (Base == 10) {
		return itoa_from_end(buf_end, value);
	}
	if constexpr (Base == 16) {
		return itoa_from_end_hex(buf_end, value);
	}
	return nullptr;
}

template <typename T, u32 Base> static u32 count_digits_base_x(T value) {
	if constexpr (Base == 2) {
		return itoa_bit_count(value);
	}
	if constexpr (Base == 8) {
		return itoa_octal_digit_count(value);
	}
	if constexpr (Base == 10) {
		return itoa_digit_count(value);
	}
	if constexpr (Base == 16) {
		return itoa_hex_digit_count(value);
	}
	return 0;
}

template <typename T, u32 Base>
static void format_integral(CharBuffer& buf, T value, const FormatOptions& opts, char sign) {
	if (sign == '\0') {
		static constexpr StackArray<T, 128> powers = gen_powers<T, Base, 0>();

		if (value >= powers[opts.width]) {
			const u32 len = count_digits_base_x<T, Base>(value);
			buf.grow(len);
			itoa_from_end_base_x<T, Base>(buf.end(), value);
			return;
		}
		char* begin = buf.grow(opts.width);
		if (opts.align == '>') {
			const char* before_num = itoa_from_end_base_x<T, Base>(buf.end(), value);
			memset(begin, opts.fill, static_cast<usize>(before_num - begin));
			return;
		}
		const u32 len = count_digits_base_x<T, Base>(value);
		const u32 pad = opts.width - len;
		if (opts.align == '<') {
			itoa_from_end_base_x<T, Base>(begin + len, value);
			memset(begin + len, opts.fill, pad);
		} else {
			memset(begin, opts.fill, opts.width);
			itoa_from_end_base_x<T, Base>(begin + len + pad / 2, value);
		}
	} else {
		static constexpr StackArray<T, 128> powers = gen_powers<T, Base, 1>();

		if (value >= powers[opts.width]) {
			const u32 len = count_digits_base_x<T, Base>(value);
			buf.grow(len);
			*itoa_from_end_base_x<T, Base>(buf.end(), value) = sign;
			return;
		}
		char* begin = buf.grow(opts.width);
		if (opts.align == '>') {
			char* before_num = itoa_from_end_base_x<T, Base>(buf.end(), value) - 1;
			*before_num			 = sign;
			memset(begin, opts.fill, static_cast<usize>(before_num - begin));
			return;
		}
		const u32 len = count_digits_base_x<T, Base>(value) + 1;
		const u32 pad = opts.width - len;
		if (opts.align == '<') {
			char* before_num = itoa_from_end_base_x<T, Base>(begin + len, value) - 1;
			*before_num			 = sign;
			memset(begin + len, opts.fill, pad);
		} else {
			memset(begin, opts.fill, opts.width);
			char* before_num = itoa_from_end_base_x<T, Base>(begin + len + pad / 2, value) - 1;
			*before_num			 = sign;
		}
	}
}

// unicode          UTF-8
// [0x00000, 0x000080) 0yyyzzzz
// [0x00080, 0x000800) 110xxxyy 10yyzzzz
// [0x00800, 0x010000) 1110wwww 10xxxxyy 10yyzzzz
// [0x10000, 0x110000) 11110uvv 10vvwwww 10xxxxyy 10yyzzzz
template <typename T> static void format_char(CharBuffer& buf, T value) {
	if constexpr (sizeof(T) == 1) {
		buf.append(static_cast<char>(value));
		return;
	}
	static constexpr u32 mask	 = 0b111111;
	const u32						 utf32 = static_cast<u32>(value);
	if (value < 0x80) {
		buf.append(static_cast<char>(value));
	}
	if (utf32 < 0x800) {
		char* str = buf.grow(2);
		str[0]		= static_cast<char>(0b110 | (utf32 >> 6));
		str[1]		= static_cast<char>(0b10 | (utf32 & mask));
	}
	if (utf32 < 0x10000) {
		char* str = buf.grow(3);
		str[0]		= static_cast<char>(0b1110 | (utf32 >> 12));
		str[1]		= static_cast<char>(0b10 | ((utf32 >> 6) & mask));
		str[2]		= static_cast<char>(0b10 | (utf32 & mask));
	}
	char* str = buf.grow(4);
	str[0]		= static_cast<char>(0b11110 | (utf32 >> 18));
	str[1]		= static_cast<char>(0b10 | ((utf32 >> 12) & mask));
	str[2]		= static_cast<char>(0b10 | ((utf32 >> 6) & mask));
	str[3]		= static_cast<char>(0b10 | (utf32 & mask));
}

template <typename T> static void format_char_debug(CharBuffer& buf, T value) {
	if (value < 0x20 || value == 0x7F) {
		switch (value) {
		case '\0':
			buf.append("'\\0'");
			break;
		case '\n':
			buf.append("'\\n'");
			break;
		case '\r':
			buf.append("'\\r'");
			break;
		case '\t':
			buf.append("'\\t'");
			break;
		case '\v':
			buf.append("'\\v'");
			break;
		default:
			const u32 len = itoa_hex_digit_count(value);
			void*			str = buf.grow(len + 4);
			memcpy(str, "'\\x", 3);
			char* end = buf.end() - 1;
			itoa_from_end_hex(end, value);
			*end = '\'';
			break;
		}
		return;
	}
	if constexpr (sizeof(T) == 1) {
		char* str = buf.grow(3);
		str[0]		= '\'';
		str[1]		= static_cast<char>(value);
		str[2]		= '\'';
		return;
	}
	static constexpr u32 mask	 = 0b111111;
	const u32						 utf32 = static_cast<u32>(value);
	if (value < 0x80) {
		char* str = buf.grow(3);
		str[0]		= '\'';
		str[1]		= static_cast<char>(value);
		str[2]		= '\'';
	}
	if (utf32 < 0x800) {
		char* str = buf.grow(4);
		str[0]		= '\'';
		str[1]		= static_cast<char>(0b110 | (utf32 >> 6));
		str[2]		= static_cast<char>(0b10 | (utf32 & mask));
		str[3]		= '\'';
	}
	if (utf32 < 0x10000) {
		char* str = buf.grow(5);
		str[0]		= '\'';
		str[1]		= static_cast<char>(0b1110 | (utf32 >> 12));
		str[2]		= static_cast<char>(0b10 | ((utf32 >> 6) & mask));
		str[3]		= static_cast<char>(0b10 | (utf32 & mask));
		str[4]		= '\'';
	}
	char* str = buf.grow(6);
	str[0]		= '\'';
	str[1]		= static_cast<char>(0b11110 | (utf32 >> 18));
	str[2]		= static_cast<char>(0b10 | ((utf32 >> 12) & mask));
	str[3]		= static_cast<char>(0b10 | ((utf32 >> 6) & mask));
	str[4]		= static_cast<char>(0b10 | (utf32 & mask));
	str[5]		= '\'';
}

void Formatter<bool>::format(CharBuffer& buf, bool value) const {
	FormatOptions opts{.width = width_, .fill = fill_, .align = align_};
	if (type_ == 's') {
		format_string_view(buf, value ? StringView("true") : StringView("false"), opts);
	} else {
		format_string_view(buf, value ? StringView("1") : StringView("0"), opts);
	}
}

template <Integral T> void Formatter<T>::format(CharBuffer& buf, T value) const {
	FormatOptions opts{
			.width = width_,
			.fill	 = fill_,
			.align = align_,
	};

	char sign = sign_;

	if (value < 0) {
		value = static_cast<T>(-value);
		sign	= '-';
	}
	using UnsignedType = MakeUnsigned<T>;
	auto u_val				 = static_cast<UnsignedType>(value);

	switch (type_) {
	default:
		[[likely]] format_integral<UnsignedType, 10>(buf, u_val, opts, sign);
		break;
	case 'x':
		format_integral<UnsignedType, 16>(buf, u_val, opts, sign);
		break;
	case 'b':
		format_integral<UnsignedType, 2>(buf, u_val, opts, sign);
		break;
	case 'o':
		format_integral<UnsignedType, 8>(buf, u_val, opts, sign);
		break;
	case 'c':
		format_char(buf, u_val);
		break;
	case '?':
		format_char_debug(buf, u_val);
		break;
	}
}

template void Formatter<signed char>::format(CharBuffer& buf, signed char value) const;
template void Formatter<wchar_t>::format(CharBuffer& buf, wchar_t value) const;
template void Formatter<char8_t>::format(CharBuffer& buf, char8_t value) const;
template void Formatter<char16_t>::format(CharBuffer& buf, char16_t value) const;
template void Formatter<char32_t>::format(CharBuffer& buf, char32_t value) const;

template void Formatter<unsigned char>::format(CharBuffer& buf, unsigned char value) const;
template void Formatter<unsigned short>::format(CharBuffer& buf, unsigned short value) const;
template void Formatter<unsigned int>::format(CharBuffer& buf, unsigned int value) const;
template void Formatter<unsigned long>::format(CharBuffer& buf, unsigned long value) const;
template void Formatter<unsigned long long>::format(CharBuffer&				 buf,
																										unsigned long long value) const;

template void Formatter<char>::format(CharBuffer& buf, char value) const;
template void Formatter<short>::format(CharBuffer& buf, short value) const;
template void Formatter<int>::format(CharBuffer& buf, int value) const;
template void Formatter<long>::format(CharBuffer& buf, long value) const;
template void Formatter<long long>::format(CharBuffer& buf, long long value) const;

#ifdef BASE_INT128_FOUND
template void Formatter<i128>::format(CharBuffer& buf, i128 value) const;
template void Formatter<u128>::format(CharBuffer& buf, u128 value) const;
#endif

} // namespace base
