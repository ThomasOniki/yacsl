#pragma once

#ifdef __clang__
#define YACSL_GCC_ATTR(...)
#define YACSL_CLANG_ATTR(...) __VA_ARGS__
#define YACSL_MSVC_ATTR(...)
#elifdef __GNUC__
#define YACSL_GCC_ATTR(...) __VA_ARGS__
#define YACSL_CLANG_ATTR(...)
#define YACSL_MSVC_ATTR(...)
#elifdef _MSC_VER
#define YACSL_GCC_ATTR(...)
#define YACSL_CLANG_ATTR(...)
#define YACSL_MSVC_ATTR(...) __VA_ARGS__
#endif

#define YACSL_GCC_OR_CLANG_ATTR(...) __VA_ARGS__

#define YACSL_LIFETIMEBOUND YACSL_CLANG_ATTR([[clang::lifetimebound]])
#define YACSL_NOINLINE YACSL_GCC_OR_CLANG_ATTR([[gnu::noinline]])
#define YACSL_ALWAYS_INLINE YACSL_GCC_OR_CLANG_ATTR([[gnu::always_inline]])

#if defined(_MSC_VER) && !defined(__clang__)
#define YACSL_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define YACSL_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
