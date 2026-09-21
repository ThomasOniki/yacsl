#pragma once

#include "base/meta/type_identity.hpp"

namespace base::utils {

template <typename To>
constexpr To implicit_cast(meta::TypeIdentity<To> value) noexcept {
  return value;
}

} // namespace base::utils
