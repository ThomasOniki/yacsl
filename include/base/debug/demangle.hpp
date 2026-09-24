#pragma once

#include "base/containers/string.hpp"
#include "base/export.hpp"
#include "base/ranges/string_view.hpp"

namespace base::debug {

containers::String demangle(ranges::StringView mangled) BASE_API;

} // namespace base::debug
