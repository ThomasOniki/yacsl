#pragma once

#include "yacsl/containers/nttp_string.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/fmt/char_encodings.hpp"
#include "yacsl/fmt/debug_string.hpp"
#include "yacsl/fmt/format_string_parser.hpp"
#include "yacsl/fmt/itoa.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/limits.hpp"
#include "yacsl/meta/sign.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"
#include <string.h>

namespace yacsl::fmt {

struct PaddingOptions {
  alignas(8) u32 width;
  char                      fill;
  FormatStringParser::Align align;
};

template <meta::Character T>
void pad_char(containers::String& buf, T chr, PaddingOptions opts) {
  u32 const len = char_len(chr);
  if (len >= opts.width) {
    write_char(buf.grow(len), chr);
    return;
  }
  char*     begin      = buf.grow(opts.width);
  u32 const pad_amount = opts.width - len;
  if (opts.align == FormatStringParser::Align::Right) {
    write_char(begin + pad_amount, chr);
    memset(begin, opts.fill, pad_amount);
  } else if (opts.align == FormatStringParser::Align::Left) {
    write_char(begin, chr);
    memset(begin + len, opts.fill, pad_amount);
  } else {
    memset(begin, opts.fill, opts.width);
    write_char(begin + pad_amount / 2, chr);
  }
}

template <meta::Character T>
void pad_debug_char(containers::String& buf, T chr, PaddingOptions opts) {
  u32 const len = debug_char_len(chr);
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
  if (opts.align == FormatStringParser::Align::Right) {
    u32 const pad_amount = opts.width - len;
    str[pad_amount]      = '\'';
    ++str;
    str[pad_amount + len] = '\'';
    write_debug_char(str + pad_amount, chr);
    memset(str, opts.fill, pad_amount);
  } else if (opts.align == FormatStringParser::Align::Left) {
    u32 const pad_amount = opts.width - len;
    str[0]               = '\'';
    ++str;
    str[len] = '\'';
    write_debug_char(str, chr);
    memset(str + len, opts.fill, pad_amount);
  } else {
    u32 const pad_amount = (opts.width - len) / 2;
    str[pad_amount]      = '\'';
    ++str;
    str[pad_amount + len] = '\'';
    memset(str, opts.fill, opts.width);
    write_debug_char(str + pad_amount, chr);
  }
}

template <bool IsDebug, meta::Character T>
void pad_string_left_aligned(containers::String&               buf,
                             ranges::BasicStringView<T, usize> str,
                             PaddingOptions                    opts) noexcept {
  usize const original_size = buf.len();
  write_maybe_debug_string<IsDebug>(buf, str);

  usize const real_len = buf.len() - original_size;

  if (real_len >= opts.width) {
    return;
  }

  u32 const pad_amount = opts.width - static_cast<u32>(real_len);
  memset(buf.grow(pad_amount), opts.fill, pad_amount);
}

template <bool IsDebug, meta::Character T>
void pad_string_right_aligned(containers::String&               buf,
                              ranges::BasicStringView<T, usize> str,
                              PaddingOptions                    opts) noexcept {
  usize const real_len = maybe_debug_string_len<IsDebug>(str);
  if (opts.width <= real_len) {
    write_maybe_debug_string<IsDebug>(buf, str);
    return;
  }
  char*     begin = buf.grow(opts.width);
  u32 const pad   = static_cast<u32>(opts.width - real_len);
  memset(begin, opts.fill, pad);
  write_maybe_debug_string<IsDebug>(begin + pad, str);
}

template <bool IsDebug, meta::Character T>
void pad_string_middle_aligned(containers::String&               buf,
                               ranges::BasicStringView<T, usize> str,
                               PaddingOptions opts) noexcept {
  usize const real_len = maybe_debug_string_len<IsDebug>(str);
  if (opts.width <= real_len) {
    write_maybe_debug_string<IsDebug>(buf, str);
    return;
  }
  char*     begin = buf.grow(opts.width);
  u32 const pad   = static_cast<u32>(opts.width - real_len);
  memset(begin, opts.fill, opts.width);
  write_maybe_debug_string<IsDebug>(begin + pad / 2, str);
}

template <bool IsDebug = false, meta::Character T>
void pad_string(containers::String& buf, ranges::BasicStringView<T, usize> str,
                PaddingOptions opts) noexcept {
  if constexpr (IsDebug) {
    // + 2 for the 2 quotes
    if (str.len() + 2 > opts.width) {
      write_maybe_debug_string<IsDebug>(buf, str);
      return;
    }
  } else {
    if (str.len() > opts.width) {
      write_maybe_debug_string<IsDebug>(buf, str);
      return;
    }
  }

  if (opts.align == FormatStringParser::Align::Left) {
    pad_string_left_aligned<IsDebug>(buf, str, opts);
  } else if (opts.align == FormatStringParser::Align::Right) {
    pad_string_right_aligned<IsDebug>(buf, str, opts);
  } else {
    pad_string_middle_aligned<IsDebug>(buf, str, opts);
  }
}

template <u32 Base, usize Offset, meta::UnsignedIntegral T>
constexpr containers::StackArray<T, 129 + Offset> gen_powers() noexcept {
  containers::StackArray<T, 129 + Offset> ret = {};
  T                                       pow = Base;

  for (u32 i = Offset + 1; i < ret.len(); ++i) {
    ret[i] = pow;
    if (pow <= meta::Limits<T>::MAX / Base) {
      pow = static_cast<T>(pow * Base);
    } else {
      pow = meta::Limits<T>::MAX;
    }
  }
  return ret;
}

template <containers::NTTPString Charset = "0123456789",
          containers::NTTPString Prefix  = "", meta::UnsignedIntegral T>
void pad_integral(containers::String& buf, T value,
                  PaddingOptions opts) noexcept {
  // Prefix.len() zeroes, 0, Base, Base^2, Base^3... Base^128
  static constexpr containers::StackArray<T, 129 + Prefix.len()> powers =
      gen_powers<Charset.len(), Prefix.len(), T>();

  constexpr u32 max_len = count_digits(meta::Limits<T>::MAX, Charset.len());

  // If value >= powers[opts.width], value has width or more digits. If it does,
  // you should skip padding and just use itoa.
  if (opts.width < max_len && value >= powers[opts.width]) {
    char* str = buf.grow(count_digits(value, Charset.len()) + Prefix.len());
    memory::copy(Prefix.slice(), str);
    itoa_from_end(buf.end(), value, Charset.view());
    return;
  }

  char* begin = buf.grow(opts.width);
  if (opts.align == FormatStringParser::Align::Right) {
    char* prefix_begin =
        itoa_from_end(buf.end(), value, Charset.view()) - Prefix.len();
    memory::copy(Prefix.slice(), prefix_begin);
    memset(begin, opts.fill, static_cast<usize>(prefix_begin - begin));
  } else if (opts.align == FormatStringParser::Align::Left) {
    usize const len = count_digits(value, Charset.len());
    memory::copy(Prefix.slice(), begin);
    char* num_end = begin + Prefix.len() + len;
    itoa_from_end(num_end, value, Charset.view());
    memset(num_end, opts.fill, static_cast<usize>(buf.end() - num_end));
  } else {
    memset(begin, opts.fill, opts.width);
    usize const len     = count_digits(value, Charset.len());
    usize       padding = opts.width - (len + Prefix.len());
    padding /= 2;
    memory::copy(Prefix.slice(), begin + padding);
    itoa_from_end(begin + padding + Prefix.len() + len, value, Charset.view());
  }
}

template <containers::NTTPString Charset, containers::NTTPString Prefix = "",
          meta::SignedIntegral T>
void pad_integral(containers::String& buf, T value,
                  PaddingOptions opts) noexcept {
  if (value >= 0) {
    pad_integral<Charset, Prefix>(
        buf, static_cast<meta::MakeUnsigned<T>>(value), opts);
    return;
  }
  pad_integral<Charset, containers::cat_nttpstrings<Prefix, "-">>(
      buf, static_cast<meta::MakeUnsigned<T>>(-value), opts);
}

} // namespace yacsl::fmt
