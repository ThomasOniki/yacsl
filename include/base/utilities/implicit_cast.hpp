#pragma once

#include "base/metaprogramming/type_identity.hpp"

namespace base {

template <typename To> constexpr To implicit_cast(TypeIdentity<To> value) noexcept { return value; }

} // namespace base
