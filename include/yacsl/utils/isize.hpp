#pragma once

#include "yacsl/type_aliases.hpp"
#include "yacsl/type_traits/sign.hpp"

namespace yacsl::utils {
using isize = type_traits::MakeSigned<usize>;
} // namespace yacsl::utils
