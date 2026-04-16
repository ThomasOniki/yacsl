#pragma once

#include "base/type_traits/qualifiers.hpp"

namespace base {

// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
template <typename T> [[nodiscard]] constexpr RemoveReference<T>&& move(T&& value) {
	return static_cast<RemoveReference<T>&&>(value);
}

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
template <typename T> [[nodiscard]] constexpr T&& forward(RemoveReference<T>&& value) {
	return static_cast<T&&>(value);
}
// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
template <typename T> [[nodiscard]] constexpr T&& forward(RemoveReference<T>& value) {
	return static_cast<T&&>(value);
}

} // namespace base
