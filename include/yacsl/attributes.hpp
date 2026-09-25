#pragma once

#ifdef __clang__
#define YACSL_LIFETIMEBOUND [[clang::lifetimebound]]
#define YACSL_NOINLINE [[clang::noinline]]
#define YACSL_ALWAYS_INLINE [[clang::always_inline]]
#elifdef __GNUC__
#define YACSL_LIFETIMEBOUND
#define YACSL_NOINLINE [[gnu::noinline]]
#define YACSL_ALWAYS_INLINE [[gnu::always_inline]]
#else
#define YACSL_LIFETIMEBOUND
#define YACSL_NOINLINE
#define YACSL_ALWAYS_INLINE
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define YACSL_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define YACSL_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
