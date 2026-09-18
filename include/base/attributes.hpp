#pragma once

#ifdef __clang__
#define BASE_LIFETIMEBOUND [[clang::lifetimebound]]
#define BASE_NOINLINE [[clang::noinline]]
#define BASE_ALWAYS_INLINE [[clang::always_inline]]
#elifdef __GNUC__
#define BASE_LIFETIMEBOUND
#define BASE_NOINLINE [[gnu::noinline]]
#define BASE_ALWAYS_INLINE [[gnu::always_inline]]
#else
#define BASE_LIFETIMEBOUND
#define BASE_NOINLINE
#define BASE_ALWAYS_INLINE
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define BASE_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define BASE_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
