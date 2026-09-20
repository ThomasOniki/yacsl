#pragma once

#include "base/type_aliases.hpp"
#include "base/type_traits/sign.hpp"

namespace base {
using isize = MakeSigned<usize>;
} // namespace base
