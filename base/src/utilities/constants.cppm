export module base.utilities.constants;

import base.utilities.type_aliases;

export namespace base {

inline constexpr usize KiB = 1024;
inline constexpr usize MiB = 1024 * KiB;
inline constexpr usize GiB = 1024 * MiB;

template <typename T, T Value> inline constexpr T constant_value = Value;

} // namespace base
