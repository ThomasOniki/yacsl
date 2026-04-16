#pragma once

namespace base {

template <typename T> T& declval() noexcept {
	static_assert(false, "declval must be called only in unevaluated contexts.");
}

} // namespace base
