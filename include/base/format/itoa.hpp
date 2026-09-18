#pragma once

#include "base/attributes.hpp"
#include "base/containers/string.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/sign.hpp"

namespace base {

template <UnsignedIntegral T>
constexpr char* itoa_from_end(BASE_LIFETIMEBOUND char* buf_end, T value,
                              StringView charset = "0123456789") noexcept {
  u32 const base = static_cast<u32>(charset.size());
  while (value >= base) {
    *--buf_end = charset[static_cast<u32>(value % base)];
    value      = static_cast<T>(value / base);
  }
  *--buf_end = charset[static_cast<u32>(value)];
  return buf_end;
}

template <SignedIntegral T>
constexpr char* itoa_from_end(BASE_LIFETIMEBOUND char* buf_end, T value,
                              StringView charset = "0123456789") noexcept {
  if (value >= 0) {
    return itoa_from_end(buf_end, static_cast<MakeUnsigned<T>>(value), charset);
  }
  buf_end =
      itoa_from_end(buf_end, static_cast<MakeUnsigned<T>>(-value), charset);
  *--buf_end = '-';
  return buf_end;
}

template <UnsignedIntegral T>
constexpr u32 count_digits(T value, u32 base = 10) noexcept {
  u32 count = 1;
  while (value >= base) {
    ++count;
    value = static_cast<T>(value / base);
  }
  return count;
}
template <SignedIntegral T>
constexpr u32 count_digits(T value, u32 base = 10) noexcept {
  if (value >= 0) {
    return count_digits(static_cast<MakeUnsigned<T>>(value), base);
  }
  return count_digits(static_cast<MakeUnsigned<T>>(-value), base) + 1;
}

template <Integral T>
constexpr void itoa(char* buf, T value,
                    StringView charset = "0123456789") noexcept {
  u32 const len = count_digits(value, static_cast<u32>(charset.size()));
  itoa_from_end(buf + len, value, charset);
}
template <Integral T>
constexpr void itoa(String& buf, T value,
                    StringView charset = "0123456789") noexcept {
  u32 const len = count_digits(value, static_cast<u32>(charset.size()));
  buf.grow(len);
  itoa_from_end(buf.end(), value, charset);
}

} // namespace base
