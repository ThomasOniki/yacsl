#include <limits.h>

static_assert(CHAR_BIT == 8);
// NOLINTBEGIN(google-runtime-int)
static_assert(sizeof(short) == 2);
static_assert(sizeof(int) == 4);
static_assert(sizeof(long long) == 8);
static_assert(sizeof(float) == 4);
static_assert(sizeof(double) == 8);
// NOLINTEND(google-runtime-int)

static_assert(static_cast<int>(-1U) == -1, "libbase assumes 2's complement.");

#ifndef __has_builtin
#error __has_builtin is undefined.
#endif
