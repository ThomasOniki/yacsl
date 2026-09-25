#pragma once

#include "yacsl/type_aliases.hpp"

namespace yacsl::utils {

inline constexpr usize KiB = 1024;
inline constexpr usize MiB = 1024 * KiB;
inline constexpr usize GiB = 1024 * MiB;

template <typename T, T Value> inline constexpr T constant_value = Value;

} // namespace yacsl::utils
