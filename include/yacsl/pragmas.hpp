#pragma once

#ifdef __clang__
#define YACSL_GCC_OR_CLANG_PRAGMA(...) _Pragma(#__VA_ARGS__)
#define YACSL_CLANG_PRAGMA(...) _Pragma(#__VA_ARGS__)
#define YACSL_GCC_PRAGMA(...)
#elifdef __GNUC__
#define YACSL_GCC_OR_CLANG_PRAGMA(...) _Pragma(#__VA_ARGS__)
#define YACSL_CLANG_PRAGMA(...)
#define YACSL_YACSL_GCC_PRAGMA(...) _Pragma(#__VA_ARGS__)
#else
#define BASEYACSL_GCC_OR_CLANG_PRAGMA(...)
#define YACSL_CLANG_PRAGMA(...)
#define YACSL_GCC_PRAGMA(...)
#endif
