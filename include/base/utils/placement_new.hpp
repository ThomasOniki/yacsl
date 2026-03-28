#pragma once

#include "base/utils/size_t.hpp"

#ifdef BASE_NO_LIBCPP
[[nodiscard]] constexpr void* operator new(base::size_t, void* ptr) noexcept { return ptr; }
#else
#include <new>
#endif
