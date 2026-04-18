#pragma once

namespace base {

#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER)
#define BASE_INT128_FOUND
#endif

#ifdef BASE_INT128_FOUND
using i128 = __int128_t;
using u128 = __uint128_t;
#endif

} // namespace base
