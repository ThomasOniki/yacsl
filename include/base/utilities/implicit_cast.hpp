#pragma once

#include "base/meta/type_identity.hpp"

namespace base {

template <typename To>
constexpr To implicit_cast(TypeIdentity<To> value) noexcept {
  return value;
}

} // namespace base
