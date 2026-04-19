#pragma once

#include "base/type_traits/sign.hpp"
#include <stdint.h>

namespace base {

using u8	= uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8	= int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

using uchar = unsigned char;

using usize = decltype(sizeof(int));
using isize = MakeSigned<usize>;
using uptr	= uintptr_t;
using iptr	= intptr_t;

// String litterals are pointers, you can get ptrdiff by substracting them.
using ptrdiff = decltype("" - "");

} // namespace base
