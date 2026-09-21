#pragma once

#include "base/containers/string.hpp"
#include "base/ranges/string_view.hpp"

namespace base::utils {

containers::String demangle(ranges::StringView mangled);

} // namespace base::utils
