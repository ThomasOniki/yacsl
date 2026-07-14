#pragma once

#ifdef __clang__
#define BASE_GCC_OR_CLANG_PRAGMA(...) _Pragma(#__VA_ARGS__)
#define BASE_CLANG_PRAGMA(...) _Pragma(#__VA_ARGS__)
#define BASE_GCC_PRAGMA(...)
#elif defined(__GNUC__)
#define BASE_GCC_OR_CLANG_PRAGMA(...) _Pragma(#__VA_ARGS__)
#define BASE_CLANG_PRAGMA(...)
#define BASE_BASE_GCC_PRAGMA(...) _Pragma(#__VA_ARGS__)
#else
#define BASEBASE_GCC_OR_CLANG_PRAGMA(...)
#define BASE_CLANG_PRAGMA(...)
#define BASE_GCC_PRAGMA(...)
#endif
