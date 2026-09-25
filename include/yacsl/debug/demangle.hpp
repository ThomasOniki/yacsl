#pragma once

#include "yacsl/containers/string.hpp"
#include "yacsl/export.hpp"
#include "yacsl/ranges/string_view.hpp"

namespace yacsl::debug {

containers::String demangle(ranges::StringView mangled) YACSL_API;

} // namespace yacsl::debug
