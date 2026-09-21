#pragma once

#include "base/type_aliases.hpp"
#include "base/type_traits/sign.hpp"

namespace base::utils {
using isize = type_traits::MakeSigned<usize>;
} // namespace base::utils
