#pragma once

#include "yacsl/meta/type_identity.hpp"

namespace yacsl::utils {

template <typename To>
constexpr To implicit_cast(meta::TypeIdentity<To> value) noexcept {
  return value;
}

} // namespace yacsl::utils
