#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::fmt {

constexpr u32 utf8_char_len(u8 utf8) noexcept {
  if (utf8 <= 127) {
    return 1;
  }
  if (utf8 >> 5U == 0b110) {
    return 2;
  }
  if (utf8 >> 4U == 0b1110) {
    return 3;
  }
  if (utf8 >> 3U == 0b11110) {
    return 4;
  }
  return 0;
}

// unicode             UTF-8
// [0x00000, 0x000080) 0yyyzzzz
// [0x00080, 0x000800) 110xxxyy 10yyzzzz
// [0x00800, 0x010000) 1110wwww 10xxxxyy 10yyzzzz
// [0x10000, 0x110000) 11110uvv 10vvwwww 10xxxxyy 10yyzzzz
inline char* write_char1(YACSL_LIFETIMEBOUND char* buf, char8_t chr) {
  *buf = static_cast<char>(chr);
  return buf + 1;
}
inline char* write_char2(YACSL_LIFETIMEBOUND char* buf, char32_t chr) {
  static constexpr char32_t mask = 0b11'1111;
  buf[0] = static_cast<char>(0b1100'0000U | (chr >> 6U));
  buf[1] = static_cast<char>(0b1000'0000U | (chr & mask));
  return buf + 2;
}
inline char* write_char3(YACSL_LIFETIMEBOUND char* buf, char32_t chr) {
  static constexpr char32_t mask = 0b11'1111;
  buf[0] = static_cast<char>(0b1110'0000U | (chr >> 12U));
  buf[1] = static_cast<char>(0b1000'0000U | ((chr >> 6U) & mask));
  buf[2] = static_cast<char>(0b1000'0000U | (chr & mask));
  return buf + 3;
}
inline char* write_char4(YACSL_LIFETIMEBOUND char* buf, char32_t chr) {
  static constexpr char32_t mask = 0b11'1111;
  buf[0] = static_cast<char>(0b1111'0000U | (chr >> 18U));
  buf[1] = static_cast<char>(0b1000'0000U | ((chr >> 12U) & mask));
  buf[2] = static_cast<char>(0b1000'0000U | ((chr >> 6U) & mask));
  buf[3] = static_cast<char>(0b1000'0000U | (chr & mask));
  return buf + 4;
}

// [U+0000, U+D800) and [U+E000, U+10000)
// Unicode == UTF-16
//
// [U+010000, U+110000)
// Unicode - 0x10000 = yyyyyyyyyyxxxxxxxxxx
// W1 = 0xD800 + yyyyyyyyyy = 110110yyyyyyyyyy
// W2 = 0xDC00 + xxxxxxxxxx = 110111xxxxxxxxxx
inline char* write_low_surrogate(YACSL_LIFETIMEBOUND char* buf, char32_t high,
                                 char32_t low) {
  static constexpr char32_t mask = 0b11'1111'1111;
  high &= mask;
  low &= mask;
  char32_t const unicode = 0x10000 + ((high << 10U) | low);
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
template <meta::Character C>
char* write_char(YACSL_LIFETIMEBOUND char* buf, C chr) {
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

template <meta::Character C> void write_char(containers::String& buf, C chr) {
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

template <meta::Character C> u32 char_len(C chr) {
  if constexpr (sizeof(C) == 1) {
    return 1;
  }
  if constexpr (sizeof(C) == 2) {
    if (chr < 0x80) {
      return 1;
    }
    if (chr < 0xD800) {
      return chr < 0x800 ? 2 : 3;
    }
    if (chr >= 0xE000) {
      return 3;
    }
    // If its a surrogate, returns 2 here. All surrogate pairs in UTF-8 are 4
    // chars, and when the 2 2s from the 2 surrogates are added it makes 4.
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

template <meta::Character C>
usize real_string_len(ranges::BasicStringView<C, usize> str) {
  if constexpr (sizeof(C) == 1) {
    return str.size();
  }
  usize ret = 0;
  for (C const chr : str) {
    ret += char_len(chr);
  }
  return ret;
}

template <meta::Character C>
void write_string(char* buf, ranges::BasicStringView<C, usize> str) {
  if constexpr (sizeof(C) == 1) {
    memory::copy(str.bytes(), reinterpret_cast<u8*>(buf));
    return;
  }
  for (C const chr : str) {
    buf = write_char(buf, chr);
  }
}

template <meta::Character C>
void write_string(containers::String&               buf,
                  ranges::BasicStringView<C, usize> str) {
  if constexpr (sizeof(C) == 1) {
    memory::copy(str.bytes(), reinterpret_cast<u8*>(buf.grow(str.size())));
    return;
  }
  for (C const chr : str) {
    write_char(buf, chr);
  }
}

} // namespace yacsl::fmt
