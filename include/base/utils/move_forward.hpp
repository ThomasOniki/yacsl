#pragma once

#include "base/utils/remove_x.hpp"

namespace base {

template <typename T> [[nodiscard]] constexpr RemoveReference<T>&& move(T&& value) { return static_cast<RemoveReference<T>&&>(value); }

template <typename T> [[nodiscard]] constexpr T&& forward(RemoveReference<T>&& value) { return static_cast<T&&>(value); }
template <typename T> [[nodiscard]] constexpr T&& forward(RemoveReference<T>& value) { return static_cast<T&&>(value); }

} // namespace base
