module;

#include "base/attributes.hpp"
#include <string.h>

export module base.formatting.formatter;

import base.type_traits.categories;
import base.type_traits.limits;
import base.type_traits.type_string;
import base.type_traits.range_like;
import base.type_traits.sign;
import base.type_traits.type_string;
import base.type_traits.qualifiers;
import base.formatting.format_string_parser;
import base.containers.string;
import base.containers.stack_array;
import base.containers.nttp_string;
import base.utilities.type_aliases;
import base.memory.lifetimes;
import base.ranges.string_view;
import base.metaprogramming.integer_sequence;
import base.metaprogramming.declval;
import base.strings.itoa;

namespace base {

// unicode             UTF-8
// [0x00000, 0x000080) 0yyyzzzz
// [0x00080, 0x000800) 110xxxyy 10yyzzzz
// [0x00800, 0x010000) 1110wwww 10xxxxyy 10yyzzzz
// [0x10000, 0x110000) 11110uvv 10vvwwww 10xxxxyy 10yyzzzz
char* write_char1(BASE_LIFETIMEBOUND char* buf, char8_t chr) {
  *buf = static_cast<char>(chr);
  return buf + 1;
}
char* write_char2(BASE_LIFETIMEBOUND char* buf, char32_t chr) {
  static constexpr char32_t mask = 0b11'1111;
  buf[0]                         = static_cast<char>(0b1100'0000 | (chr >> 6));
  buf[1]                         = static_cast<char>(0b1000'0000 | (chr & mask));
  return buf + 2;
}
char* write_char3(BASE_LIFETIMEBOUND char* buf, char32_t chr) {
  static constexpr char32_t mask = 0b11'1111;
  buf[0]                         = static_cast<char>(0b1110'0000 | (chr >> 12));
  buf[1]                         = static_cast<char>(0b1000'0000 | ((chr >> 6) & mask));
  buf[2]                         = static_cast<char>(0b1000'0000 | (chr & mask));
  return buf + 3;
}
char* write_char4(BASE_LIFETIMEBOUND char* buf, char32_t chr) {
  static constexpr char32_t mask = 0b11'1111;
  buf[0]                         = static_cast<char>(0b1111'0000 | (chr >> 18));
  buf[1]                         = static_cast<char>(0b1000'0000 | ((chr >> 12) & mask));
  buf[2]                         = static_cast<char>(0b1000'0000 | ((chr >> 6) & mask));
  buf[3]                         = static_cast<char>(0b1000'0000 | (chr & mask));
  return buf + 4;
}

// [U+0000, U+D800) and [U+E000, U+10000)
// Unicode == UTF-16
//
// [U+010000, U+110000)
// Unicode - 0x10000 = yyyyyyyyyyxxxxxxxxxx
// W1 = 0xD800 + yyyyyyyyyy = 110110yyyyyyyyyy
// W2 = 0xDC00 + xxxxxxxxxx = 110111xxxxxxxxxx
char* write_low_surrogate(BASE_LIFETIMEBOUND char* buf, char32_t high, char32_t low) {
  static constexpr char32_t mask = 0b11'1111'1111;
  high &= mask;
  low &= mask;
  const char32_t unicode = 0x10000 + ((high << 10) | low);
  return write_char4(buf, unicode);
}

// unicode             UTF-8
// [0x00000, 0x000080) 0yyyzzzz
// [0x00080, 0x000800) 110xxxyy 10yyzzzz
// [0x00800, 0x010000) 1110wwww 10xxxxyy 10yyzzzz
// [0x10000, 0x110000) 11110uvv 10vvwwww 10xxxxyy 10yyzzzz
//
// UTF-16:
// [U+0000, U+D800) and [U+E000, U+10000)
// Unicode == UTF-16
//
// [010000, U+110000)
// Unicode - 0x10000 = yyyyyyyyyyxxxxxxxxxx
// W1 = 0xD800 + yyyyyyyyyy = 110110yyyyyyyyyy
// W2 = 0xDC00 + xxxxxxxxxx = 110111xxxxxxxxxx
template <typename C> char* write_char(BASE_LIFETIMEBOUND char* buf, C chr) {
  if (sizeof(C) == 1 || chr < 0x80) {
    return write_char1(buf, static_cast<char8_t>(chr));
  }
  if (chr < 0x800) {
    return write_char2(buf, static_cast<char32_t>(chr));
  }
  if (sizeof(C) == 2 && chr >= 0xD800 && chr < 0xE000) {
    static thread_local char32_t high;
    if (chr < 0xDC00) {
      high = static_cast<char32_t>(chr);
      return buf;
    }
    return write_low_surrogate(buf, high, static_cast<char32_t>(chr));
  }
  if (chr < 0x10000) {
    return write_char3(buf, static_cast<char32_t>(chr));
  }
  return write_char4(buf, static_cast<char32_t>(chr));
}

template <typename C> void write_char(String& buf, C chr) {
  if (sizeof(C) == 1 || chr < 0x80) {
    write_char1(buf.grow(1), static_cast<char8_t>(chr));
  } else if (chr < 0x800) {
    write_char2(buf.grow(2), static_cast<char32_t>(chr));
  } else if (sizeof(C) == 2 && chr >= 0xD800 && chr < 0xE000) {
    static thread_local char32_t high;
    if (chr < 0xDC00) {
      high = static_cast<char32_t>(chr);
    } else {
      write_low_surrogate(buf.grow(4), high, static_cast<char32_t>(chr));
    }
  } else if (chr < 0x10000) {
    write_char3(buf.grow(3), static_cast<char32_t>(chr));
  } else {
    write_char4(buf.grow(4), static_cast<char32_t>(chr));
  }
}

template <typename T> u32 char_len(T chr) {
  if constexpr (sizeof(T) == 1) {
    return 1;
  }
  if constexpr (sizeof(T) == 2) {
    if (chr < 0x80) {
      return 1;
    }
    if (chr < 0xD800) {
      return chr < 0x800 ? 2 : 3;
    }
    if (chr >= 0xE000) {
      return 3;
    }
    // If its a surrogate, returns 2 here. All surrogate pairs in UTF-8 are 4 chars, and when the
    // 2 2s from the 2 surrogates are added it makes 4.
    return 2;
  }
  if (chr < 0x80) {
    return 1;
  }
  if (chr < 0x800) {
    return 2;
  }
  if (chr < 0x10000) {
    return 3;
  }
  return 4;
}

template <typename T> u32 debug_char_len(T chr) {
  if (chr == '\n' || chr == '\0' || chr == '\r' || chr == '\t' || chr == '\v') {
    return 2;
  }
  if (chr < 0x20 || chr == 0x7F) {
    return chr >= 0x10 ? 3 : 2;
  }
  return char_len(chr);
}

template <bool Debug, typename C> usize real_string_len(BasicStringView<C, usize> str) {
  if constexpr (Debug) {
    usize len = 2;
    for (const C chr : str) {
      len += debug_char_len(chr);
    }
    return len;
  }
  if constexpr (sizeof(C) == 1) {
    return str.size();
  }
  usize ret = 0;
  for (const C chr : str) {
    ret += char_len(chr);
  }
  return ret;
}

char* write_debug_char_below_0x20(BASE_LIFETIMEBOUND char* buf, char chr) {
  buf[0] = '\\';
  switch (chr) {
  case '\n':
    buf[1] = 'n';
    return buf + 2;
  case '\0':
    buf[1] = '0';
    return buf + 2;
  case '\t':
    buf[1] = 't';
    return buf + 2;
  case '\r':
    buf[1] = 'r';
    return buf + 2;
  case '\v':
    buf[1] = 'v';
    return buf + 2;
  default:
    buf[1] = 'x';
    if (chr < 0x10) {
      static constexpr StringView hex_digits = "0123456789abcdef";
      buf[2]                                 = hex_digits[static_cast<uchar>(chr)];
      return buf + 3;
    }
    static constexpr StringView table = "101112131415161718191a1b1c1d1e1f";
    copy_bytes(buf + 2, &table[static_cast<usize>(chr - 0x10) * 2], 2);
    return buf + 4;
  }
}

template <typename C> char* write_debug_char(BASE_LIFETIMEBOUND char* buf, C chr) {
  // For some reason clang-tidy's warning about casting [signed] char to unsigned types other than
  // uchar also applies to wchar_t, but you can't cast a wchar to uchar like it suggests if you
  // don't want to lose precision.
  // NOLINTNEXTLINE(bugprone-signed-char-misuse, cert-str34-c)
  auto uchr = static_cast<MakeUnsigned<C>>(chr);
  if (uchr >= 0x20) {
    if (uchr == 0x7F) {
      buf[0] = '\\';
      buf[1] = '\\';
      buf[2] = '7';
      buf[3] = 'F';
      return buf + 4;
    }
    return write_char(buf, uchr);
  }
  return write_debug_char_below_0x20(buf, static_cast<char>(uchr));
}

void write_debug_char_below_0x20(String& buf, char chr) {
  char* str = nullptr;
  switch (chr) {
  case '\n':
    str    = buf.grow(2);
    str[0] = '\\';
    str[1] = 'n';
    break;
  case '\0':
    str    = buf.grow(2);
    str[0] = '\\';
    str[1] = '0';
    break;
  case '\t':
    str    = buf.grow(2);
    str[0] = '\\';
    str[1] = 't';
    break;
  case '\r':
    str    = buf.grow(2);
    str[0] = '\\';
    str[1] = 'r';
    break;
  case '\v':
    str    = buf.grow(2);
    str[0] = '\\';
    str[1] = 'v';
    break;
  default:
    if (chr < 0x10) {
      static constexpr StringView hex_digits = "0123456789abcdef";
      str                                    = buf.grow(3);
      str[0]                                 = '\\';
      str[1]                                 = 'x';
      str[2]                                 = hex_digits[static_cast<uchar>(chr)];
    } else {
      static constexpr StringView table = "101112131415161718191a1b1c1d1e1f";
      str                               = buf.grow(4);
      str[0]                            = '\\';
      str[1]                            = 'x';
      copy_bytes(str + 2, &table[static_cast<usize>(chr - 0x10) * 2], 2);
    }
  }
}

template <typename C> void write_debug_char(String& buf, C chr) {
  // For some reason clang-tidy's warning about casting [signed] char to unsigned types other than
  // uchar also applies to wchar_t, but you can't cast a wchar to uchar like it suggests if you
  // don't want to lose precision.
  // NOLINTNEXTLINE(bugprone-signed-char-misuse, cert-str34-c)
  auto uchr = static_cast<MakeUnsigned<C>>(chr);
  if (uchr >= 0x20) {
    if (uchr == 0x7F) {
      copy_bytes(buf.grow(4), "\\x7F", 4);
    } else {
      write_char(buf, uchr);
    }
  } else {
    write_debug_char_below_0x20(buf, static_cast<char>(uchr));
  }
}

struct FormatOptions {
  alignas(8) u32 width;
  char fill;
  char align;
};

template <typename C> void format_char(String& buf, C chr, FormatOptions opts) {
  const u32 len = char_len(chr);
  if (len >= opts.width) {
    write_char(buf.grow(len), chr);
    return;
  }
  char*     begin      = buf.grow(opts.width);
  const u32 pad_amount = opts.width - len;
  if (opts.align == '>') {
    write_char(begin + pad_amount, chr);
    memset(begin, opts.fill, pad_amount);
  } else if (opts.align == '<') {
    write_char(begin, chr);
    memset(begin + len, opts.fill, pad_amount);
  } else {
    memset(begin, opts.fill, opts.width);
    write_char(begin + pad_amount / 2, chr);
  }
}

template <typename C> void format_debug_char(String& buf, C chr, FormatOptions opts) {
  const u32 len = debug_char_len(chr);
  // +2 for the quotes
  if (len + 2 >= opts.width) {
    char* str = buf.grow(len + 2);
    str[0]    = '\'';
    ++str;
    str[len] = '\'';
    write_debug_char(str, chr);
    return;
  }
  char* str = buf.grow(opts.width);
  if (opts.align == '>') {
    const u32 pad_amount = opts.width - len;
    str[pad_amount]      = '\'';
    ++str;
    str[pad_amount + len] = '\'';
    write_debug_char(str + pad_amount, chr);
    memset(str, opts.fill, pad_amount);
  } else if (opts.align == '<') {
    const u32 pad_amount = opts.width - len;
    str[0]               = '\'';
    ++str;
    str[len] = '\'';
    write_debug_char(str, chr);
    memset(str + len, opts.fill, pad_amount);
  } else {
    const u32 pad_amount = (opts.width - len) / 2;
    str[pad_amount]      = '\'';
    ++str;
    str[pad_amount + len] = '\'';
    memset(str, opts.fill, opts.width);
    write_debug_char(str + pad_amount, chr);
  }
}

template <bool Debug, typename C> void write_string(char* buf, BasicStringView<C, usize> str) {
  if constexpr (Debug) {
    *buf = '"';
    ++buf;
    for (const C chr : str) {
      buf = write_debug_char(buf, chr);
    }
    *buf = '"';
    return;
  }
  if constexpr (sizeof(C) == 1) {
    copy_bytes(buf, str.data(), str.size());
    return;
  }
  for (const C chr : str) {
    buf = write_char(buf, chr);
  }
}

template <bool Debug, typename C> void write_string(String& buf, BasicStringView<C, usize> str) {
  if constexpr (Debug) {
    buf += '"';
    for (const C chr : str) {
      write_debug_char(buf, chr);
    }
    buf += '"';
    return;
  }
  if constexpr (sizeof(C) == 1) {
    copy_bytes(buf.grow(str.size()), str.data(), str.size());
    return;
  }
  for (const C chr : str) {
    write_char(buf, chr);
  }
}

template <bool Debug, typename C>
void format_string_left_aligned(String& buf, BasicStringView<C, usize> str,
                                FormatOptions opts) noexcept {
  const usize original_size = buf.size();
  write_string<Debug>(buf, str);

  const usize real_len = buf.size() - original_size;

  if (real_len >= opts.width) {
    return;
  }

  const u32 pad_amount = opts.width - static_cast<u32>(real_len);
  memset(buf.grow(pad_amount), opts.fill, pad_amount);
}

template <bool Debug, typename C>
void format_string_right_aligned(String& buf, BasicStringView<C, usize> str,
                                 FormatOptions opts) noexcept {
  const usize real_len = real_string_len<Debug>(str);
  if (opts.width <= real_len) {
    write_string<Debug>(buf, str);
    return;
  }
  char*     begin = buf.grow(opts.width);
  const u32 pad   = opts.width - real_len;
  memset(begin, opts.fill, pad);
  write_string<Debug>(begin + pad, str);
}

template <bool Debug, typename C>
void format_string_middle_aligned(String& buf, BasicStringView<C, usize> str,
                                  FormatOptions opts) noexcept {
  const usize real_len = real_string_len<Debug>(str);
  if (opts.width <= real_len) {
    write_string<Debug>(buf, str);
    return;
  }
  char*     begin = buf.grow(opts.width);
  const u32 pad   = opts.width - real_len;
  memset(begin, opts.fill, opts.width);
  write_string<Debug>(begin + pad / 2, str);
}

template <bool Debug, typename C>
void format_string(String& buf, BasicStringView<C, usize> str, FormatOptions opts) noexcept {
  if constexpr (Debug) {
    // + 2 for the 2 quotes
    if (str.size() + 2 > opts.width) {
      write_string<Debug>(buf, str);
      return;
    }
  } else {
    if (str.size() > opts.width) {
      write_string<Debug>(buf, str);
      return;
    }
  }

  if (opts.align == '<') {
    format_string_left_aligned<Debug>(buf, str, opts);
  } else if (opts.align == '>') {
    format_string_right_aligned<Debug>(buf, str, opts);
  } else {
    format_string_middle_aligned<Debug>(buf, str, opts);
  }
}

template <typename T, u32 Base>
char* itoa_from_end_base_x(BASE_LIFETIMEBOUND char* buf_end, T value) {
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

template <typename T, u32 Base> u32 count_digits_base_x(T value) {
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

template <UnsignedIntegral T, T Base> constexpr StackArray<T, 130> gen_powers() noexcept {
  StackArray<T, 130> ret = {};
  T                  pow = Base;

  for (u32 i = 2; i < 130; ++i) {
    ret[i] = pow;
    if (pow <= Limits<T>::max / Base) {
      pow = static_cast<T>(pow * Base);
    } else {
      pow = Limits<T>::max;
    }
  }
  return ret;
}

template <u32 Base, UnsignedIntegral T>
void format_integral(String& buf, T value, FormatOptions opts, char sign) noexcept {
  // 0, 0, Base, Base^2, Base^3...
  static constexpr StackArray<T, 130> powers = gen_powers<T, Base>();

  const bool has_sign = sign != '\0';

  // If value < powers[width], value has width or less digits. If it does, you can skip counting
  // digits and format from buf + width if the alignment is right (which is the default).
  // The - has_sign is here because if there is a sign to print, the value  can be Base times
  // smaller since one of the digits will replaced by the sign.
  // The + 1 is here because the whole powers array is offset. This is so that if width is 0 and
  // has_sign is 1, you don't get -1 as the index.
  if (value >= powers[1 + opts.width - has_sign]) {
    const u32 len = count_digits_base_x<T, Base>(value) + has_sign;
    buf.grow(len);
    char* before_num = itoa_from_end_base_x<T, Base>(buf.end(), value);
    if (has_sign) {
      --before_num;
      *before_num = sign;
    }
    return;
  }

  char* begin = buf.grow(opts.width);
  if (opts.align == '>') {
    char* before_num = itoa_from_end_base_x<T, Base>(buf.end(), value);
    if (has_sign) {
      --before_num;
      *before_num = sign;
    }
    memset(begin, opts.fill, static_cast<usize>(before_num - begin));
    return;
  }
  const u32 len = count_digits_base_x<T, Base>(value) + has_sign;
  if (opts.align == '<') {
    const u32 pad = opts.width - len;
    itoa_from_end_base_x<T, Base>(begin + len, value);
    if (has_sign) {
      *begin = sign;
    }
    memset(begin + len, opts.fill, pad);
  } else { // middle align
    const u32 pad = (opts.width - len) / 2;
    memset(begin, opts.fill, opts.width);
    itoa_from_end_base_x<T, Base>(begin + len + pad, value);
    if (has_sign) {
      begin[pad] = sign;
    }
  }
}

export template <typename T> class Formatter;

template <Integral T> class Formatter<T> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('>');
    sign_  = parser.check_sign();
    width_ = parser.check_width();
    type_  = parser.check_option(Character<T> ? 'c' : 'd', "bodxc?");
    return parser.end_parse();
  }

  void format(String& buf, T value) const noexcept {
    const FormatOptions opts{
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };

    char sign = sign_;

    if (value < 0) {
      value = static_cast<T>(-value);
      sign  = '-';
    }
    using UnsignedType = MakeUnsigned<T>;
    // For some reason clang-tidy's warning about casting [signed] char to unsigned types other than
    // uchar also applies to wchar_t, but you can't cast a wchar to uchar like it suggests if you
    // don't want to lose precision.
    // NOLINTNEXTLINE(bugprone-signed-char-misuse, cert-str34-c)
    auto u_val = static_cast<UnsignedType>(value);

    switch (type_) {
    default:
      [[likely]] format_integral<10>(buf, u_val, opts, sign);
      break;
    case 'x':
      format_integral<16>(buf, u_val, opts, sign);
      break;
    case 'b':
      format_integral<2>(buf, u_val, opts, sign);
      break;
    case 'o':
      format_integral<8>(buf, u_val, opts, sign);
      break;
    case 'c':
      format_char(buf, u_val, opts);
      break;
    case '?':
      format_debug_char(buf, u_val, opts);
      break;
    }
  }

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '>';
  char sign_  = '\0';
  char type_  = 'd';
};

template <> class Formatter<bool> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('>');
    width_ = parser.check_width();
    type_  = parser.check_option('a', "ad");
    return parser.end_parse();
  }

  void format(String& buf, bool value) const noexcept {
    FormatOptions opts{
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };
    if (type_ == 'a') {
      format_string<false>(buf, value ? "true"_sv : "false"_sv, opts);
    } else {
      format_char(buf, '0' + static_cast<char>(value), opts);
    }
  }

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '>';
  char type_  = 'a';
};

template <FloatingPoint T> class Formatter<T> {
public:
  constexpr const char* parse(const char* fmt) {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('>');
    sign_  = parser.check_sign();
    width_ = parser.check_width();
    type_  = parser.check_option('g', "gfea");
    return parser.end_parse();
  }

  void format(String& buf, T value) const noexcept;

private:
  u32  precision_ = 0;
  u32  width_     = 0;
  char fill_      = ' ';
  char align_     = '>';
  char sign_      = '\0';
  char type_      = 'g';
};

template <typename T> class Formatter<T*> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('>');
    width_ = parser.check_width();
    return parser.end_parse();
  }

  void format(String& buf, const T* ptr) const noexcept {
    const FormatOptions opts{
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    format_integral<16>(buf, reinterpret_cast<uptr>(ptr), opts, '\0');
  }

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '<';
};

template <Character Char, Integral SizeT> class Formatter<BasicStringView<Char, SizeT>> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('<');
    width_ = parser.check_width();
    type_  = parser.check_option('s', "s?");
    return parser.end_parse();
  }

  void format(String& buf, BasicStringView<Char, SizeT> str) const noexcept {
    const FormatOptions opts{.width = width_, .fill = fill_, .align = align_};
    if (type_ == 's') [[likely]] {
      format_string<false>(buf, BasicStringView<Char>(str.data(), static_cast<usize>(str.size())),
                           opts);
    } else {
      format_string<true>(buf, BasicStringView<Char>(str.data(), static_cast<usize>(str.size())),
                          opts);
    }
  }

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '<';
  char type_  = 's';
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
template <Character Char, usize N> class Formatter<Char[N]> : Formatter<BasicStringView<Char>> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    return Formatter<BasicStringView<Char>>::parse(fmt);
  }
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  void format(String& buf, const Char (&str)[N]) const noexcept {
    return Formatter<BasicStringView<Char>>::format(
        buf, BasicStringView(static_cast<const Char*>(str), N - 1));
  }
};

template <Character Char, usize N>
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
class Formatter<const Char[N]> : Formatter<BasicStringView<Char>> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    return Formatter<BasicStringView<Char>>::parse(fmt);
  }
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  void format(String& buf, const Char (&str)[N]) const noexcept {
    return Formatter<BasicStringView<Char>>::format(buf, BasicStringView(str, N - 1));
  }
};

template <RangeLike T>
class Formatter<T> : Formatter<RemoveCVRef<decltype(*declval<T>().begin())>> {
public:
  constexpr const char* parse(const char* fmt) noexcept { return Base::parse(fmt); }

  void format(String& buf, const T& range) const noexcept {
    buf += '[';
    auto iter = range.begin();
    auto end  = range.end();
    if (iter != end) {
      Base::format(buf, *iter);
      ++iter;
      for (; iter != end; ++iter) {
        buf += ", ";
        Base::format(buf, *iter);
      }
    }
    buf += ']';
  }

private:
  using Element = RemoveCVRef<decltype(*declval<T>().begin())>;
  using Base    = Formatter<Element>;
};

template <Enum T> class Formatter<T> {
public:
  constexpr const char* parse(const char* fmt) noexcept {
    FormatStringParser parser(fmt);
    fill_  = parser.check_fill();
    align_ = parser.check_align('<');
    width_ = parser.check_width();
    return parser.end_parse();
  }

  void format(String& buf, T value) const noexcept {
    auto          scalar = static_cast<UnderlyingType>(value);
    auto          u_val  = static_cast<MakeUnsigned<UnderlyingType>>(value);
    FormatOptions opts   = {
        .width = width_,
        .fill  = fill_,
        .align = align_,
    };
    if (u_val < 256) {
      format_string<false>(buf, TABLE[u_val], opts);
      return;
    }
    static constexpr StackArray<UnderlyingType, 130> powers = gen_powers<UnderlyingType, 10>();

    if (scalar < 0) {
      u_val = static_cast<MakeUnsigned<UnderlyingType>>(-scalar);
    }

    if (u_val >= powers[1 + width_ - (scalar < 0)]) {
      const usize len = itoa_digit_count(scalar);
      char*       str = buf.grow(len + CAST_STRING.size());
      copy_bytes(str, CAST_STRING);
      itoa_from_end(buf.end(), scalar);
      return;
    }

    char* str = buf.grow(width_);
    if (align_ == '>') {
      char* end = itoa_from_end(buf.end(), scalar);
      end -= CAST_STRING.size();
      copy_bytes(end, CAST_STRING);
      memset(str, fill_, static_cast<usize>(end - str));
    } else if (align_ == '<') {
      copy_bytes(str, CAST_STRING);
      char* end = itoa(str + CAST_STRING.size(), scalar);
      memset(end, fill_, static_cast<usize>(buf.end() - end));
    } else {
      memset(str, fill_, width_);
      const usize len    = itoa_digit_count(scalar) + CAST_STRING.size();
      const usize offset = (width_ - len) / 2;
      copy_bytes(str + offset, CAST_STRING);
      itoa_from_end(str + offset + len, scalar);
    }
  }

private:
  static constexpr StackArray<char, TYPE_STRING<T>.size() + 2> CAST_STRING_ARR = []() {
    StackArray<char, TYPE_STRING<T>.size() + 2> arr{};
    arr[0]              = '(';
    arr[arr.size() - 1] = ')';
    copy_bytes(static_cast<void*>(&arr[1]), TYPE_STRING<T>);
    return arr;
  }();

  static constexpr StringView CAST_STRING{CAST_STRING_ARR.data(), CAST_STRING_ARR.size()};

  using UnderlyingType = UnderlyingEnumType<T>;
  template <u32... Values>
  static constexpr StackArray<StringView, sizeof...(Values)>
  make_table(IntegerSequence<u32, Values...> /*sequence*/) {
    StackArray<StringView, sizeof...(Values)> ret;
    (..., [&]() { ret[Values] = table_entry<T{static_cast<UnderlyingType>(Values)}>(); }());
    return ret;
  }

  template <T Value> static constexpr StringView table_entry_pretty() {
    return __PRETTY_FUNCTION__;
  }

  static constexpr usize TABLE_ENTRY_OFFSET_FROM_BEGIN = []() -> usize {
    constexpr StringView pretty  = table_entry_pretty<static_cast<T>(0)>();
    constexpr StringView to_find = "Value = ";
    for (const char* it = pretty.begin(); it < pretty.end() - to_find.size(); ++it) {
      for (usize i = 0;; ++i) {
        if (i == to_find.size()) {
          return static_cast<usize>(it - pretty.begin()) + i - "_pretty"_sv.size();
        }
        if (it[i] != to_find[i]) {
          break;
        }
      }
    }
    return 0;
  }();

  static constexpr usize TABLE_ENTRY_OFFSET_FROM_END = []() -> usize {
    constexpr StringView pretty    = table_entry_pretty<static_cast<T>(0)>();
    usize                templates = 0;
    for (usize i = TABLE_ENTRY_OFFSET_FROM_BEGIN; i < pretty.size(); ++i) {
      if (pretty[i] == '<') {
        ++templates;
      } else if (pretty[i] == '>') {
        --templates;
      }
      if (templates > 0) {
        continue;
      }
      if (pretty[i] == ',' || pretty[i] == ';' || pretty[i] == ']') {
        return pretty.size() - i;
      }
    }
    return pretty.size();
  }();

  template <T Value> static constexpr StringView table_entry() {
    constexpr StringView pretty = __PRETTY_FUNCTION__;
    return {pretty.begin() + TABLE_ENTRY_OFFSET_FROM_BEGIN,
            pretty.end() - TABLE_ENTRY_OFFSET_FROM_END};
  }

  static constexpr StackArray<StringView, 256> TABLE = make_table(IntegerSequenceToN<u32, 256>{});

private:
  u32  width_ = 0;
  char fill_  = ' ';
  char align_ = '>';
};

} // namespace base
