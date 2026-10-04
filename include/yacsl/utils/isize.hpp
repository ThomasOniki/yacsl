#pragma once

#include "yacsl/meta/sign.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::utils {
using isize = meta::MakeSigned<usize>;
} // namespace yacsl::utils
