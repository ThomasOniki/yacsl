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

// unicode             UTF-8
// [0x00000, 0x000080) 0yyyzzzz
// [0x00080, 0x000800) 110xxxyy 10yyzzzz
// [0x00800, 0x010000) 1110wwww 10xxxxyy 10yyzzzz
// [0x10000, 0x110000) 11110uvv 10vvwwww 10xxxxyy 10yyzzzz
static char* write_char1(BASE_LIFETIMEBOUND char* buf, char8_t chr) {
  *buf = static_cast<char>(chr);
  return buf + 1;
}
static char* write_char2(BASE_LIFETIMEBOUND char* buf, char32_t chr) {
  static constexpr char32_t mask = 0b11'1111;
  buf[0]                         = static_cast<char>(0b1100'0000 | (chr >> 6));
  buf[1]                         = static_cast<char>(0b1000'0000 | (chr & mask));
  return buf + 2;
}
static char* write_char3(BASE_LIFETIMEBOUND char* buf, char32_t chr) {
  static constexpr char32_t mask = 0b11'1111;
  buf[0]                         = static_cast<char>(0b1110'0000 | (chr >> 12));
  buf[1]                         = static_cast<char>(0b1000'0000 | ((chr >> 6) & mask));
  buf[2]                         = static_cast<char>(0b1000'0000 | (chr & mask));
  return buf + 3;
}
static char* write_char4(BASE_LIFETIMEBOUND char* buf, char32_t chr) {
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
static char* write_low_surrogate(BASE_LIFETIMEBOUND char* buf, char32_t high, char32_t low) {
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
template <typename C> static char* write_char(BASE_LIFETIMEBOUND char* buf, C chr) {
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

template <typename C> static void write_char(CharBuffer& buf, C chr) {
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

template <typename T> static u32 char_len(T chr) {
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

template <typename T> static u32 debug_char_len(T chr) {
  if (chr == '\n' || chr == '\0' || chr == '\r' || chr == '\t' || chr == '\v') {
    return 2;
  }
  if (chr < 0x20 || chr == 0x7F) {
    return chr >= 0x10 ? 3 : 2;
  }
  return char_len(chr);
}

template <bool Debug, typename C> static u32 real_string_len(const BasicStringView<C, u32>& str) {
  if constexpr (Debug) {
    u32 len = 2;
    for (const C chr : str) {
      len += debug_char_len(chr);
    }
    return len;
  }
  if constexpr (sizeof(C) == 1) {
    return str.size();
  }
  u32 ret = 0;
  for (const C chr : str) {
    ret += char_len(chr);
  }
  return ret;
}

static char* write_debug_char_below_0x20(BASE_LIFETIMEBOUND char* buf, char chr) {
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
    memcpy(buf + 2, &table[static_cast<usize>(chr - 0x10) * 2], 2);
    return buf + 4;
  }
}

template <typename C> static char* write_debug_char(BASE_LIFETIMEBOUND char* buf, C chr) {
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

static void write_debug_char_below_0x20(CharBuffer& buf, char chr) {
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
      memcpy(str + 2, &table[static_cast<usize>(chr - 0x10) * 2], 2);
    }
  }
}

template <typename C> static void write_debug_char(CharBuffer& buf, C chr) {
  // For some reason clang-tidy's warning about casting [signed] char to unsigned types other than
  // uchar also applies to wchar_t, but you can't cast a wchar to uchar like it suggests if you
  // don't want to lose precision.
  // NOLINTNEXTLINE(bugprone-signed-char-misuse, cert-str34-c)
  auto uchr = static_cast<MakeUnsigned<C>>(chr);
  if (uchr >= 0x20) {
    if (uchr == 0x7F) {
      memcpy(buf.grow(4), "\\x7F", 4);
    } else {
      write_char(buf, uchr);
    }
  } else {
    write_debug_char_below_0x20(buf, static_cast<char>(uchr));
  }
}

namespace {
struct FormatOptions {
  alignas(8) u32 width;
  char fill;
  char align;
};
} // namespace

template <typename C> static void format_char(CharBuffer& buf, C chr, const FormatOptions& opts) {
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

template <typename C>
static void format_debug_char(CharBuffer& buf, C chr, const FormatOptions& opts) {
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

template <bool Debug, typename C>
static void write_string(char* buf, const BasicStringView<C, u32>& str) {
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
    memcpy(buf, str.data(), str.size());
    return;
  }
  for (const C chr : str) {
    buf = write_char(buf, chr);
  }
}

template <bool Debug, typename C>
static void write_string(CharBuffer& buf, const BasicStringView<C, u32>& str) {
  if constexpr (Debug) {
    buf.append('"');
    for (const C chr : str) {
      write_debug_char(buf, chr);
    }
    buf.append('"');
    return;
  }
  if constexpr (sizeof(C) == 1) {
    memcpy(buf.grow(str.size()), str.data(), str.size());
    return;
  }
  for (const C chr : str) {
    write_char(buf, chr);
  }
}

template <bool Debug, typename C>
static void format_string(CharBuffer& buf, const BasicStringView<C, usize>& usize_str,
                          const FormatOptions& opts) {
  const BasicStringView<C, u32> str(usize_str.data(), static_cast<u32>(usize_str.size()));

  if constexpr (Debug) {
    // + 2 for the 2 quotes
    if (usize_str.size() + 2 > opts.width) {
      write_string<Debug>(buf, str);
      return;
    }
  } else {
    if (usize_str.size() > opts.width) {
      write_string<Debug>(buf, str);
      return;
    }
  }

  if (opts.align == '<') {
    const usize original_size = buf.size();
    write_string<Debug>(buf, str);
    const usize real_len = buf.size() - original_size;

    if (real_len >= opts.width) {
      return;
    }
    const u32 pad_amount = opts.width - static_cast<u32>(real_len);
    memset(buf.grow(pad_amount), opts.fill, pad_amount);
    return;
  }

  const u32 real_len = real_string_len<Debug>(str);
  if (opts.width <= real_len) {
    write_string<Debug>(buf, str);
    return;
  }
  char*     begin = buf.grow(opts.width);
  const u32 pad   = opts.width - real_len;
  if (opts.align == '>') {
    memset(begin, opts.fill, pad);
    write_string<Debug>(begin + pad, str);
  } else {
    memset(begin, opts.fill, opts.width);
    write_string<Debug>(begin + pad / 2, str);
  }
}

template <Character Char>
void Formatter<BasicStringView<Char>>::format(CharBuffer&                  buf,
                                              const BasicStringView<Char>& str) const {
  const FormatOptions opts{.width = width_, .fill = fill_, .align = align_};
  if (type_ == 's') [[likely]] {
    format_string<false>(buf, str, opts);
  } else {
    format_string<true>(buf, str, opts);
  }
}

template void Formatter<BasicStringView<char>>::format(CharBuffer&, Reference) const;
template void Formatter<BasicStringView<unsigned char>>::format(CharBuffer&, Reference) const;
template void Formatter<BasicStringView<signed char>>::format(CharBuffer&, Reference) const;
template void Formatter<BasicStringView<wchar_t>>::format(CharBuffer&, Reference) const;
template void Formatter<BasicStringView<char8_t>>::format(CharBuffer&, Reference) const;
template void Formatter<BasicStringView<char16_t>>::format(CharBuffer&, Reference) const;
template void Formatter<BasicStringView<char32_t>>::format(CharBuffer&, Reference) const;

template <typename T, u32 Base>
static char* itoa_from_end_base_x(BASE_LIFETIMEBOUND char* buf_end, T value) {
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

template <typename T, T Base> static constexpr StackArray<T, 130> gen_powers() {
  StackArray<T, 130> ret = {};
  T                  pow = Base;

  for (u32 i = 2; i < 130; ++i) {
    if (pow <= Limits<T>::max / Base) {
      pow = static_cast<T>(pow * Base);
    } else {
      pow = Limits<T>::max;
    }
    ret[i] = pow;
  }
  return ret;
}

template <u32 Base, typename T>
static void format_integral(CharBuffer& buf, T value, const FormatOptions& opts, char sign) {
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

void Formatter<bool>::format(CharBuffer& buf, bool value) const {
  const FormatOptions opts{
      .width = width_,
      .fill  = fill_,
      .align = align_,
  };
  if (type_ == 's') {
    format_string<false>(buf, value ? StringView("true") : StringView("false"), opts);
  } else {
    format_string<false>(buf, value ? StringView("1") : StringView("0"), opts);
  }
}

template <Integral T> void Formatter<T>::format(CharBuffer& buf, T value) const {
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

void Formatter<void*>::format(CharBuffer& buf, const void* ptr) const {
  const FormatOptions opts{
      .width = width_,
      .fill  = fill_,
      .align = align_,
  };
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  format_integral<16>(buf, reinterpret_cast<uptr>(ptr), opts, '\0');
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
template void Formatter<unsigned long long>::format(CharBuffer&        buf,
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
