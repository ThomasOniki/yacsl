#pragma once

#include "base/containers/string.hpp"
#include "base/ranges/string_view.hpp"

namespace base {

String demangle(StringView mangled);

} // namespace base
