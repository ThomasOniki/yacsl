#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/sign.hpp"
#include "yacsl/ranges/string_view.hpp"

namespace yacsl::fmt {

template <meta::UnsignedIntegral T>
constexpr char*
itoa_from_end(YACSL_LIFETIMEBOUND char* buf_end, T value,
              ranges::StringView charset = "0123456789") noexcept {
  u32 const base = static_cast<u32>(charset.len());
  while (value >= base) {
    *--buf_end = charset[static_cast<u32>(value % base)];
    value      = static_cast<T>(value / base);
  }
  *--buf_end = charset[static_cast<u32>(value)];
  return buf_end;
}

template <meta::SignedIntegral T>
constexpr char*
itoa_from_end(YACSL_LIFETIMEBOUND char* buf_end, T value,
              ranges::StringView charset = "0123456789") noexcept {
  if (value >= 0) {
    return itoa_from_end(buf_end, static_cast<meta::MakeUnsigned<T>>(value),
                         charset);
  }
  buf_end = itoa_from_end(buf_end, static_cast<meta::MakeUnsigned<T>>(-value),
                          charset);
  *--buf_end = '-';
  return buf_end;
}

template <meta::UnsignedIntegral T>
constexpr u32 count_digits(T value, u32 base = 10) noexcept {
  u32 count = 1;
  while (value >= base) {
    ++count;
    value = static_cast<T>(value / base);
  }
  return count;
}
template <meta::SignedIntegral T>
constexpr u32 count_digits(T value, u32 base = 10) noexcept {
  if (value >= 0) {
    return count_digits(static_cast<meta::MakeUnsigned<T>>(value), base);
  }
  return count_digits(static_cast<meta::MakeUnsigned<T>>(-value), base) + 1;
}

template <meta::Integral T>
constexpr void itoa(char* buf, T value,
                    ranges::StringView charset = "0123456789") noexcept {
  u32 const len = count_digits(value, static_cast<u32>(charset.len()));
  itoa_from_end(buf + len, value, charset);
}
template <meta::Integral T>
constexpr void itoa(containers::String& buf, T value,
                    ranges::StringView charset = "0123456789") noexcept {
  u32 const len = count_digits(value, static_cast<u32>(charset.len()));
  buf.grow(len);
  itoa_from_end(buf.end(), value, charset);
}
template <meta::Integral T>
constexpr containers::String
itoa(T value, ranges::StringView charset = "0123456789") noexcept {
  containers::String buf;
  itoa(buf, value, charset);
  return buf;
}

} // namespace yacsl::fmt
