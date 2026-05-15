#pragma once

namespace base {

constexpr bool is_constant_evaluated() noexcept { return __builtin_is_constant_evaluated(); }

} // namespace base
