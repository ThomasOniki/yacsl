#include <limits.h>

static_assert(CHAR_BIT == 8);

#if defined(BASE_NO_LIBCPP) && !defined(__has_builtin)
#error __has_builtin required for builting without libcpp.
#endif
