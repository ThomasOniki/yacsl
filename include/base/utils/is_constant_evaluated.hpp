#pragma once

namespace base::utils {

[[nodiscard]] constexpr bool is_constant_evaluated() {
  if consteval {
    return true;
  } else {
    return false;
  }
}

} // namespace base::utils
