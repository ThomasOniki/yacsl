#pragma once

#if !defined(__SIZEOF_INT128__) || defined(_MSC_VER)
#include "yacsl/containers/stack_array.hpp"
#endif

namespace yacsl::utils {
#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER)
using i128 = __int128_t;
using u128 = __uint128_t;
#else
struct i128 {
  containers::StackArray<u8, 16> dummmy;
};
using u128 = i128;
#endif

} // namespace yacsl::utils
