#pragma once

#include "base/type_traits/qualifiers.hpp"

namespace base {

template <typename T> // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
[[nodiscard]] constexpr RemoveReference<T>&& move(BASE_LIFETIMEBOUND T&& value) {
  return static_cast<RemoveReference<T>&&>(value);
}

template <typename T> // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
[[nodiscard]] constexpr T&& forward(BASE_LIFETIMEBOUND RemoveReference<T>&& value) {
  return static_cast<T&&>(value);
}
template <typename T>
[[nodiscard]] constexpr T&& forward(BASE_LIFETIMEBOUND RemoveReference<T>& value) {
  return static_cast<T&&>(value);
}

} // namespace base
