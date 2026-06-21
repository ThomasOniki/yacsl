#pragma once

#ifdef __clang__
#define BASE_LIFETIMEBOUND [[clang::lifetimebound]]
#else
#define BASE_LIFETIMEBOUND
#endif
