#pragma once

#include "base/type_traits/sign.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {
using isize = MakeSigned<usize>;
} // namespace base
