#pragma once

#include "base/attributes.hpp"
#include "base/containers/string.hpp"
#include "base/format/char_encodings.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_aliases.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/sign.hpp"

namespace base {

template <Character C> u32 debug_char_len(C chr) {
  if (chr == '\n' || chr == '\0' || chr == '\r' || chr == '\t' || chr == '\v') {
    return 2;
  }
  if (chr < 0x20 || chr == 0x7F) {
    return chr >= 0x10 ? 3 : 2;
  }
  return char_len(chr);
}

template <Character C>
usize debug_string_len(BasicStringView<C, usize> str) noexcept {
  usize len = 0;
  for (C const chr : str) {
    len += debug_char_len(chr);
  }
  return len;
}

inline char* write_debug_char_below_0x20(BASE_LIFETIMEBOUND char* buf,
                                         char                     chr) {
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
      buf[2] = hex_digits[static_cast<uchar>(chr)];
      return buf + 3;
    }
    static constexpr StringView table = "101112131415161718191a1b1c1d1e1f";
    copy({&table[static_cast<usize>(chr - 0x10) * 2], 2}, buf + 2);
    return buf + 4;
  }
}

template <Character C>
char* write_debug_char(BASE_LIFETIMEBOUND char* buf, C chr) {
  // For some reason clang-tidy's warning about casting [signed] char to
  // unsigned types other than uchar also applies to wchar_t, but you can't cast
  // a wchar to uchar like it suggests if you don't want to lose precision.
  // NOLINTNEXTLINE(bugprone-signed-char-misuse)
  auto uchr = static_cast<MakeUnsigned<C>>(chr);
  if (uchr >= 0x20) {
    if (uchr == 0x7F) {
      buf[0] = '\\';
      buf[1] = 'x';
      buf[2] = '7';
      buf[3] = 'F';
      return buf + 4;
    }
    return write_char(buf, chr);
  }
  return write_debug_char_below_0x20(buf, static_cast<char>(uchr));
}

inline void write_debug_char_below_0x20(String& buf, char chr) {
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
  case '':
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
      str[2] = hex_digits[static_cast<uchar>(chr)];
    } else {
      static constexpr StringView table = "101112131415161718191a1b1c1d1e1f";
      str                               = buf.grow(4);
      str[0]                            = '\\';
      str[1]                            = 'x';
      copy({&table[static_cast<usize>(chr - 0x10) * 2], 2}, str + 2);
    }
  }
}

template <Character C> void write_debug_char(String& buf, C chr) {
  // For some reason clang-tidy's warning about casting [signed] char to
  // unsigned types other than uchar also applies to wchar_t, but you can't cast
  // a wchar to uchar like it suggests if you don't want to lose precision.
  // NOLINTNEXTLINE(bugprone-signed-char-misuse)
  auto uchr = static_cast<MakeUnsigned<C>>(chr);
  if (uchr >= 0x20) {
    if (uchr == 0x7F) {
      copy("\x7F"_sv.slice(), buf.grow(4));
    } else {
      write_char(buf, chr);
    }
  } else {
    write_debug_char_below_0x20(buf, static_cast<char>(uchr));
  }
}

template <Character C>
void write_debug_string(char* buf, BasicStringView<C, usize> str) {
  *buf = '"';
  ++buf;
  for (C const chr : str) {
    buf = write_debug_char(buf, chr);
  }
  *buf = '"';
}
template <Character C>
void write_debug_string(String& buf, BasicStringView<C, usize> str) {
  buf += '"';
  for (C const chr : str) {
    write_debug_char(buf, chr);
  }
  buf += '"';
}

template <bool IsDebug, Character C>
void write_maybe_debug_string(String& buf, BasicStringView<C, usize> str) {
  if constexpr (IsDebug) {
    write_debug_string(buf, str);
  } else {
    write_string(buf, str);
  }
}
template <bool IsDebug, Character C>
void write_maybe_debug_string(char* buf, BasicStringView<C, usize> str) {
  if constexpr (IsDebug) {
    write_debug_string(buf, str);
  } else {
    write_string(buf, str);
  }
}

template <bool IsDebug, Character C>
usize maybe_debug_string_len(BasicStringView<C, usize> str) {
  if constexpr (IsDebug) {
    return debug_string_len(str);
  } else {
    return real_string_len(str);
  }
}

} // namespace base
