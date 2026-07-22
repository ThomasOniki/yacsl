#pragma once

#include "base/containers/nttp_string.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string.hpp"
#include "base/formatting/char_encodings.hpp"
#include "base/formatting/debug_string.hpp"
#include "base/formatting/itoa.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/limits.hpp"
#include "base/type_traits/sign.hpp"
#include "base/utilities/type_aliases.hpp"
#include <string.h>

namespace base {

struct PaddingOptions {
  alignas(8) u32 width;
  char fill;
  char align;
};

template <Character C> void pad_char(String& buf, C chr, PaddingOptions opts) {
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

template <Character C> void pad_debug_char(String& buf, C chr, PaddingOptions opts) {
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

template <bool IsDebug, Character C>
void pad_string_left_aligned(String& buf, BasicStringView<C, usize> str,
                             PaddingOptions opts) noexcept {
  const usize original_size = buf.size();
  write_maybe_debug_string<IsDebug>(buf, str);

  const usize real_len = buf.size() - original_size;

  if (real_len >= opts.width) {
    return;
  }

  const u32 pad_amount = opts.width - static_cast<u32>(real_len);
  memset(buf.grow(pad_amount), opts.fill, pad_amount);
}

template <bool IsDebug, Character C>
void pad_string_right_aligned(String& buf, BasicStringView<C, usize> str,
                              PaddingOptions opts) noexcept {
  const usize real_len = maybe_debug_string_len<IsDebug>(str);
  if (opts.width <= real_len) {
    write_maybe_debug_string<IsDebug>(buf, str);
    return;
  }
  char*     begin = buf.grow(opts.width);
  const u32 pad   = static_cast<u32>(opts.width - real_len);
  memset(begin, opts.fill, pad);
  write_maybe_debug_string<IsDebug>(begin + pad, str);
}

template <bool IsDebug, Character C>
void pad_string_middle_aligned(String& buf, BasicStringView<C, usize> str,
                               PaddingOptions opts) noexcept {
  const usize real_len = maybe_debug_string_len<IsDebug>(str);
  if (opts.width <= real_len) {
    write_maybe_debug_string<IsDebug>(buf, str);
    return;
  }
  char*     begin = buf.grow(opts.width);
  const u32 pad   = static_cast<u32>(opts.width - real_len);
  memset(begin, opts.fill, opts.width);
  write_maybe_debug_string<IsDebug>(begin + pad / 2, str);
}

template <bool IsDebug, Character C>
void pad_string(String& buf, BasicStringView<C, usize> str, PaddingOptions opts) noexcept {
  if constexpr (IsDebug) {
    // + 2 for the 2 quotes
    if (str.size() + 2 > opts.width) {
      write_maybe_debug_string<IsDebug>(buf, str);
      return;
    }
  } else {
    if (str.size() > opts.width) {
      write_maybe_debug_string<IsDebug>(buf, str);
      return;
    }
  }

  if (opts.align == '<') {
    pad_string_left_aligned<IsDebug>(buf, str, opts);
  } else if (opts.align == '>') {
    pad_string_right_aligned<IsDebug>(buf, str, opts);
  } else {
    pad_string_middle_aligned<IsDebug>(buf, str, opts);
  }
}

template <u32 Base, usize Offset, UnsignedIntegral T>
constexpr StackArray<T, 129 + Offset> gen_powers() noexcept {
  StackArray<T, 129 + Offset> ret = {};
  T                           pow = Base;

  for (u32 i = Offset + 1; i < ret.size(); ++i) {
    ret[i] = pow;
    if (pow <= Limits<T>::max / Base) {
      pow = static_cast<T>(pow * Base);
    } else {
      pow = Limits<T>::max;
    }
  }
  return ret;
}

template <u32 Base, NTTPString Prefix = "", UnsignedIntegral T>
void pad_integral(String& buf, T value, PaddingOptions opts) noexcept {
  // Prefix.size() zeroes, 0, Base, Base^2, Base^3... Base^128
  static constexpr StackArray<T, 129 + Prefix.size()> powers = gen_powers<Base, Prefix.size(), T>();

  constexpr u32 max_len = count_digits_base_x<Base>(Limits<T>::max);

  // If value < powers[width], value has width or less digits. If it does, you should skip padding
  // and just use itoa.
  if (opts.width < max_len && value >= powers[opts.width]) {
    char* str = buf.grow(count_digits_base_x<Base>(value) + Prefix.size());
    copy_bytes(str, Prefix.data(), Prefix.size());
    itoa_from_end_base_x<Base>(buf.end(), value);
    return;
  }

  char* begin = buf.grow(opts.width);
  if (opts.align == '>') {
    char* prefix_begin = itoa_from_end_base_x<Base>(buf.end(), value) - Prefix.size();
    copy_bytes(prefix_begin, Prefix.data(), Prefix.size());
    memset(begin, opts.fill, static_cast<usize>(prefix_begin - begin));
  } else if (opts.align == '<') {
    const usize len = count_digits_base_x<Base>(value);
    copy_bytes(begin, Prefix.data(), Prefix.size());
    char* num_end = begin + Prefix.size() + len;
    itoa_from_end_base_x<Base>(num_end, value);
    memset(num_end, opts.fill, static_cast<usize>(buf.end() - num_end));
  } else {
    memset(begin, opts.fill, opts.width);
    const usize len     = count_digits_base_x<Base>(value);
    usize       padding = opts.width - (len + Prefix.size());
    padding /= 2;
    copy_bytes(begin + padding, Prefix.data(), Prefix.size());
    itoa_from_end_base_x<Base>(begin + padding + Prefix.size() + len, value);
  }
}

template <u32 Base, NTTPString Prefix = "", SignedIntegral T>
void pad_integral(String& buf, T value, PaddingOptions opts) noexcept {
  if (value < 0) {
    constexpr auto with_minus = []() {
      NTTPString<Prefix.size() + 1> ret;
      copy_bytes(static_cast<void*>(ret.data()), Prefix.data(), Prefix.size());
      ret[Prefix.size()] = '-';
      return ret;
    }();
    pad_integral<Base, with_minus>(buf, static_cast<MakeUnsigned<T>>(-value), opts);
  } else {
    pad_integral<Base, Prefix>(buf, static_cast<MakeUnsigned<T>>(value), opts);
  }
}

} // namespace base
