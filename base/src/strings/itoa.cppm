module;

#include "base/attributes.hpp"
#include <string.h>

export module base.strings.itoa;

import base.utilities.type_aliases;
import base.type_traits.categories;
import base.type_traits.sign;
import base.type_traits.comparaison;
import base.type_traits.limits;
import base.math.powers;

export namespace base {

template <Integral T> constexpr char* itoa_bin(BASE_LIFETIMEBOUND char* buf, T value) noexcept;
template <Integral T> constexpr char* itoa_oct(BASE_LIFETIMEBOUND char* buf, T value) noexcept;
template <Integral T> constexpr char* itoa(BASE_LIFETIMEBOUND char* buf, T value) noexcept;
template <Integral T> constexpr char* itoa_hex(BASE_LIFETIMEBOUND char* buf, T value) noexcept;

template <UnsignedIntegral T>
constexpr char* itoa_from_end_bin(BASE_LIFETIMEBOUND char* buf_end, T value) noexcept;
template <UnsignedIntegral T>
constexpr char* itoa_from_end_oct(BASE_LIFETIMEBOUND char* buf_end, T value) noexcept;
template <UnsignedIntegral T>
constexpr char* itoa_from_end(BASE_LIFETIMEBOUND char* buf_end, T value) noexcept;
template <UnsignedIntegral T>
constexpr char* itoa_from_end_hex(BASE_LIFETIMEBOUND char* buf_end, T value) noexcept;

template <SignedIntegral T>
constexpr char* itoa_from_end_bin(BASE_LIFETIMEBOUND char* buf_end, T value) noexcept;
template <SignedIntegral T>
constexpr char* itoa_from_end_oct(BASE_LIFETIMEBOUND char* buf_end, T value) noexcept;
template <SignedIntegral T>
constexpr char* itoa_from_end(BASE_LIFETIMEBOUND char* buf_end, T value) noexcept;
template <SignedIntegral T>
constexpr char* itoa_from_end_hex(BASE_LIFETIMEBOUND char* buf_end, T value) noexcept;

template <UnsignedIntegral T> constexpr u32 itoa_bit_count(T value) noexcept;
template <UnsignedIntegral T> constexpr u32 itoa_octal_digit_count(T value) noexcept;
template <UnsignedIntegral T> constexpr u32 itoa_digit_count(T value) noexcept;
template <UnsignedIntegral T> constexpr u32 itoa_hex_digit_count(T value) noexcept;

template <SignedIntegral T> constexpr u32 itoa_bit_count(T value) noexcept;
template <SignedIntegral T> constexpr u32 itoa_octal_digit_count(T value) noexcept;
template <SignedIntegral T> constexpr u32 itoa_digit_count(T value) noexcept;
template <SignedIntegral T> constexpr u32 itoa_hex_digit_count(T value) noexcept;

template <Integral T> constexpr char* itoa_bin(char* buf, T value) noexcept {
  u32 len = itoa_bit_count(value);
  itoa_from_end_bin(buf + len, value);
  return buf + len;
}
template <Integral T> constexpr char* itoa_oct(char* buf, T value) noexcept {
  u32 len = itoa_octal_digit_count(value);
  itoa_from_end_oct(buf + len, value);
  return buf + len;
}
template <Integral T> constexpr char* itoa(char* buf, T value) noexcept {
  u32 len = itoa_digit_count(value);
  itoa_from_end(buf + len, value);
  return buf + len;
}
template <Integral T> constexpr char* itoa_hex(char* buf, T value) noexcept {
  u32 len = itoa_hex_digit_count(value);
  itoa_from_end_hex(buf + len, value);
  return buf + len;
}

template <SignedIntegral T> constexpr u32 itoa_bit_count(T value) noexcept {
  if (value < 0) {
    return 1 + itoa_bit_count(static_cast<MakeUnsigned<T>>(-value));
  }
  return itoa_bit_count(static_cast<MakeUnsigned<T>>(value));
}
template <SignedIntegral T> constexpr u32 itoa_octal_digit_count(T value) noexcept {
  if (value < 0) {
    return 1 + itoa_octal_digit_count(static_cast<MakeUnsigned<T>>(-value));
  }
  return itoa_octal_digit_count(static_cast<MakeUnsigned<T>>(value));
}
template <SignedIntegral T> constexpr u32 itoa_digit_count(T value) noexcept {
  if (value < 0) {
    return 1 + itoa_digit_count(static_cast<MakeUnsigned<T>>(-value));
  }
  return itoa_digit_count(static_cast<MakeUnsigned<T>>(value));
}
template <SignedIntegral T> constexpr u32 itoa_hex_digit_count(T value) noexcept {
  if (value < 0) {
    return 1 + itoa_hex_digit_count(static_cast<MakeUnsigned<T>>(-value));
  }
  return itoa_hex_digit_count(static_cast<MakeUnsigned<T>>(value));
}

// NOLINTBEGIN(google-runtime-int)
template <UnsignedIntegral T> constexpr u32 itoa_bit_count(T value) noexcept {
  if (value == 0) {
    return 1;
  }
#ifdef BASE_INT128_FOUND
  if constexpr (SameAs<T, u128>) {
    static_assert(sizeof(unsigned long long) == sizeof(u64));
    if (value < Limits<u64>::max) [[likely]] {
      return Limits<u64>::max_bits -
             static_cast<u32>(__builtin_clzll(static_cast<unsigned long long>(value)));
    }
    return Limits<u128>::max_bits -
           static_cast<u32>(__builtin_clzll(static_cast<unsigned long long>(value >> 64)));
  } else
#endif
      if constexpr (SameAs<T, unsigned long long>) {
    return Limits<T>::max_bits - static_cast<u32>(__builtin_clzll(value));
  } else if constexpr (SameAs<T, unsigned long>) {
    return Limits<T>::max_bits - static_cast<u32>(__builtin_clzl(value));
  } else {
    return Limits<int>::max_bits - static_cast<u32>(__builtin_clz(value));
  }
}

// This divides by 4 because 4 bits is one hex digit (2^4 == 16).
template <UnsignedIntegral T> constexpr u32 itoa_hex_digit_count(T value) noexcept {
  if (value == 0) {
    return 1;
  }
#ifdef BASE_INT128_FOUND
  if constexpr (SameAs<T, u128>) {
    static_assert(sizeof(unsigned long long) == sizeof(u64));
    if (value < Limits<u64>::max) [[likely]] {
      return Limits<u64>::max_bits / 4 -
             static_cast<u32>(__builtin_clzll(static_cast<unsigned long long>(value))) / 4;
    }
    return Limits<u128>::max_bits / 4 -
           static_cast<u32>(__builtin_clzll(static_cast<unsigned long long>(value >> 64))) / 4;
  } else
#endif
      if constexpr (SameAs<T, unsigned long long>) {
    return Limits<T>::max_bits / 4 - static_cast<u32>(__builtin_clzll(value)) / 4;
  } else if constexpr (SameAs<T, unsigned long>) {
    return Limits<T>::max_bits / 4 - static_cast<u32>(__builtin_clzl(value)) / 4;
  } else {
    return Limits<int>::max_bits / 4 - static_cast<u32>(__builtin_clz(value)) / 4;
  }
}

template <UnsignedIntegral T> constexpr u32 itoa_octal_digit_count(T value) noexcept {
  if (value == 0) {
    return 1;
  }
  u32 bits = 0;
#ifdef BASE_INT128_FOUND
  if constexpr (SameAs<T, u128>) {
    static_assert(sizeof(unsigned long long) == sizeof(u64));
    if (value < Limits<u64>::max) [[likely]] {
      const auto ull = static_cast<unsigned long long>(value);
      bits           = Limits<u64>::max_bits - static_cast<u32>(__builtin_clzll(ull));
    } else {
      const auto ull = static_cast<unsigned long long>(value >> 64);
      bits           = Limits<u128>::max_bits - static_cast<u32>(__builtin_clzll(ull));
    }
  } else
#endif
      if constexpr (SameAs<T, unsigned long long>) {
    bits = Limits<T>::max_bits - static_cast<u32>(__builtin_clzll(value));
  } else if constexpr (SameAs<T, unsigned long>) {
    bits = Limits<T>::max_bits - static_cast<u32>(__builtin_clzl(value));
  } else {
    bits = Limits<int>::max_bits - static_cast<u32>(__builtin_clz(value));
  }
  // This divides by 3 for the same reason. The +2s are to round up.
  return (bits + 2) / 3;
}
// NOLINTEND(google-runtime-int)

// NOLINTNEXTLINE(readability-function-cognitive-complexity, google-readability-function-size)
template <UnsignedIntegral T> constexpr u32 itoa_digit_count(T value) noexcept {
  // clang-format off
	if (value < pow10<T>(1)) { return 1; }
	if (value < pow10<T>(2)) { return 2; }
	if constexpr (sizeof(T) <= 1) { return 3; }
	if (value < pow10<T>(3)) { return 3; }
	if (value < pow10<T>(4)) { return 4; }
	if constexpr (sizeof(T) <= 2) { return 6; }
	if (value < pow10<T>(5)) { return 5; }
	if (value < pow10<T>(6)) { return 6; }
	if (value < pow10<T>(7)) { return 7; }
	if (value < pow10<T>(8)) { return 8; }
	if (value < pow10<T>(9)) { return 9; }
	if constexpr (sizeof(T) <= 4) { return 10; }
	if (value < pow10<T>(10)) { return 10; }
	if (value < pow10<T>(11)) { return 11; }
	if (value < pow10<T>(12)) { return 12; }
	if (value < pow10<T>(13)) { return 13; }
	if (value < pow10<T>(14)) { return 14; }
	if (value < pow10<T>(15)) { return 15; }
	if (value < pow10<T>(16)) { return 16; }
	if (value < pow10<T>(17)) { return 17; }
	if (value < pow10<T>(18)) { return 18; }
	if (value < pow10<T>(19)) { return 19; }
	if constexpr (sizeof(T) <= 8) { return 20; }
	if (value < pow10<T>(20)) { return 20; }
	if (value < pow10<T>(21)) { return 21; }
	if (value < pow10<T>(22)) { return 22; }
	if (value < pow10<T>(23)) { return 23; }
	if (value < pow10<T>(24)) { return 24; }
	if (value < pow10<T>(25)) { return 25; }
	if (value < pow10<T>(26)) { return 26; }
	if (value < pow10<T>(27)) { return 27; }
	if (value < pow10<T>(28)) { return 28; }
	if (value < pow10<T>(29)) { return 29; }
	if (value < pow10<T>(30)) { return 30; }
	if (value < pow10<T>(31)) { return 31; }
	if (value < pow10<T>(32)) { return 32; }
	if (value < pow10<T>(33)) { return 33; }
	if (value < pow10<T>(34)) { return 34; }
	if (value < pow10<T>(35)) { return 35; }
	if (value < pow10<T>(36)) { return 36; }
	if (value < pow10<T>(37)) { return 37; }
	if (value < pow10<T>(38)) { return 38; }
	return 39;
  // clang-format on
}

template <SignedIntegral T> constexpr char* itoa_from_end_bin(char* buf_end, T value) noexcept {
  if (value < 0) {
    buf_end = itoa_from_end_bin(buf_end, static_cast<MakeUnsigned<T>>(-value));
    --buf_end;
    *buf_end = '-';
    return buf_end;
  }
  return itoa_from_end_bin(buf_end, static_cast<MakeUnsigned<T>>(value));
}
template <SignedIntegral T> constexpr char* itoa_from_end_oct(char* buf_end, T value) noexcept {
  if (value < 0) {
    buf_end = itoa_from_end_oct(buf_end, static_cast<MakeUnsigned<T>>(-value));
    --buf_end;
    *buf_end = '-';
    return buf_end;
  }
  return itoa_from_end_oct(buf_end, static_cast<MakeUnsigned<T>>(value));
}
template <SignedIntegral T> constexpr char* itoa_from_end(char* buf_end, T value) noexcept {
  if (value < 0) {
    buf_end = itoa_from_end(buf_end, static_cast<MakeUnsigned<T>>(-value));
    --buf_end;
    *buf_end = '-';
    return buf_end;
  }
  return itoa_from_end(buf_end, static_cast<MakeUnsigned<T>>(value));
}
template <SignedIntegral T> constexpr char* to_chars_hex(char* buf_end, T value) {
  if (value < 0) {
    buf_end = itoa_from_end_hex(buf_end, static_cast<MakeUnsigned<T>>(-value));
    --buf_end;
    *buf_end = '-';
    return buf_end;
  }
  return itoa_from_end_hex(buf_end, static_cast<MakeUnsigned<T>>(value));
}

template <UnsignedIntegral T> constexpr char* itoa_from_end_bin(char* buf_end, T value) noexcept {
  constexpr const char* table = "000000000001000010000011000100000101000110000111"
                                "001000001001001010001011001100001101001110001111"
                                "010000010001010010010011010100010101010110010111"
                                "011000011001011010011011011100011101011110011111"
                                "100000100001100010100011100100100101100110100111"
                                "101000101001101010101011101100101101101110101111"
                                "110000110001110010110011110100110101110110110111"
                                "111000111001111010111011111100111101111110111111";

  while (value >= 64) {
    buf_end -= 6;
    memcpy(buf_end, table + (value % 64) * 6, 6);
    value /= 64;
  }

  if (value >= 8) {
    if (value >= 16) {
      if (value >= 32) {
        buf_end -= 6;
        memcpy(buf_end, table + value * 6, 6);
      } else {
        buf_end -= 5;
        memcpy(buf_end, table + 1 + value * 6, 5);
      }
    } else {
      buf_end -= 4;
      memcpy(buf_end, table + 2 + value * 6, 4);
    }
  } else {
    if (value >= 2) {
      if (value >= 4) {
        buf_end -= 3;
        memcpy(buf_end, table + 3 + value * 6, 3);
      } else {
        buf_end -= 2;
        memcpy(buf_end, table + 4 + value * 6, 2);
      }
    } else {
      --buf_end;
      *buf_end = static_cast<char>('0' + value);
    }
  }
  return buf_end;
}

template <UnsignedIntegral T> constexpr char* itoa_from_end_oct(char* buf_end, T value) noexcept {
  constexpr const char* table = "0001020304050607"
                                "1011121314151617"
                                "2021222324252627"
                                "3031323334353637"
                                "4041424344454647"
                                "5051525354555657"
                                "6061626364656667"
                                "7071727374757677";

  while (value >= 64) {
    buf_end -= 2;
    memcpy(buf_end, table + (value % 64) * 2, 2);
    value /= 64;
  }
  if (value >= 16) {
    buf_end -= 2;
    memcpy(buf_end, table + value * 2, 2);
  } else {
    --buf_end;
    *buf_end = static_cast<char>('0' + value);
  }
  return buf_end;
}

template <UnsignedIntegral T> constexpr char* itoa_from_end(char* buf_end, T value) noexcept {
  constexpr const char* table = "00010203040506070809"
                                "10111213141516171819"
                                "20212223242526272829"
                                "30313233343536373839"
                                "40414243444546474849"
                                "50515253545556575859"
                                "60616263646566676869"
                                "70717273747576777879"
                                "80818283848586878889"
                                "90919293949596979899";

  while (value >= 100) {
    buf_end -= 2;
    memcpy(buf_end, table + (value % 100) * 2, 2);
    value /= 100;
  }

  if (value >= 10) {
    buf_end -= 2;
    memcpy(buf_end, table + value * 2, 2);
  } else {
    --buf_end;
    *buf_end = static_cast<char>('0' + value);
  }
  return buf_end;
}

template <UnsignedIntegral T> constexpr char* itoa_from_end_hex(char* buf_end, T value) noexcept {
  constexpr const char* table = "000102030405060708090a0b0c0d0e0f"
                                "101112131415161718191a1b1c1d1e1f"
                                "202122232425262728292a2b2c2d2e2f"
                                "303132333435363738393a3b3c3d3e3f"
                                "404142434445464748494a4b4c4d4e4f"
                                "505152535455565758595a5b5c5d5e5f"
                                "606162636465666768696a6b6c6d6e6f"
                                "707172737475767778797a7b7c7d7e7f"
                                "808182838485868788898a8b8c8d8e8f"
                                "909192939495969798999a9b9c9d9e9f"
                                "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
                                "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
                                "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
                                "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
                                "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
                                "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";

  constexpr const char* hexits = "0123456789abcdef";

  while (value >= 256) {
    buf_end -= 2;
    memcpy(buf_end, table + (value % 256) * 2, 2);
    value = static_cast<T>(value / 256);
  }
  if (value >= 16) {
    buf_end -= 2;
    memcpy(buf_end, table + value * 2, 2);
  } else {
    --buf_end;
    *buf_end = hexits[value];
  }
  return buf_end;
}

} // namespace base
