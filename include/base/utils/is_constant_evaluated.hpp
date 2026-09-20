#pragma once

namespace base {

[[nodiscard]] constexpr bool is_constant_evaluated() {
  if consteval {
    return true;
  } else {
    return false;
  }
}

} // namespace base
