#pragma once

#ifdef __clang__
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BASE_ATTRIBUTE(attr) [[clang::attr]]
#elif defined(__GNUC__)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BASE_ATTRIBUTE(attr) [[gnu::attr]]
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BASE_ATTRIBUTE(attr)
#endif

#define BASE_LIFETIMEBOUND BASE_ATTRIBUTE(lifetimebound)
#define BASE_NOINLINE BASE_ATTRIBUTE(noinline)
#define BASE_ALWAYS_INLINE BASE_ATTRIBUTE(always_inline)
