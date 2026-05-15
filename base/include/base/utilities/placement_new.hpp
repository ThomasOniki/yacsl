#pragma once

#ifdef BASE_NO_LIBCPP
#include "base/utilities/type_aliases.hpp"
[[nodiscard]] constexpr void* operator new(base::usize /*count*/, void* ptr) noexcept {
	return ptr;
}
#else
// NOLINTNEXTLINE(misc-include-cleaner)
#include <new>
#endif
