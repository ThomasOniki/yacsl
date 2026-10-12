#pragma once

namespace yacsl {

using u8  = __UINT8_TYPE__;
using u16 = __UINT16_TYPE__;
using u32 = __UINT32_TYPE__;
using u64 = __UINT64_TYPE__;

using i8  = __INT8_TYPE__;
using i16 = __INT16_TYPE__;
using i32 = __INT32_TYPE__;
using i64 = __INT64_TYPE__;

using f32 = float;
using f64 = double;

using usize   = __SIZE_TYPE__;
using uptr    = __UINTPTR_TYPE__;
using iptr    = __INTPTR_TYPE__;
using ptrdiff = __PTRDIFF_TYPE__;

constexpr u8 operator""_u8(unsigned long long value) noexcept {
  return static_cast<u8>(value);
}
constexpr u16 operator""_u16(unsigned long long value) noexcept {
  return static_cast<u16>(value);
}
constexpr u32 operator""_u32(unsigned long long value) noexcept {
  return static_cast<u32>(value);
}
constexpr u64 operator""_u64(unsigned long long value) noexcept {
  return static_cast<u64>(value);
}
constexpr i8 operator""_i8(unsigned long long value) noexcept {
  return static_cast<i8>(value);
}
constexpr i16 operator""_i16(unsigned long long value) noexcept {
  return static_cast<i16>(value);
}
constexpr i32 operator""_i32(unsigned long long value) noexcept {
  return static_cast<i32>(value);
}
constexpr i64 operator""_i64(unsigned long long value) noexcept {
  return static_cast<i64>(value);
}

} // namespace yacsl
