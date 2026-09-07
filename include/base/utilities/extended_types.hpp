#pragma once

namespace base {

#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER)
using i128 = __int128_t;
using u128 = __uint128_t;
#else
#include "base/containers/stack_array.hpp"
struct i128 {
  StackArray<char, 16> dummmy;
};
using u128 = i128;
#endif

} // namespace base
