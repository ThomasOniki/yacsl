#pragma once

#include "yacsl/compiler/export.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/ranges/string_view.hpp"

namespace yacsl::debug {

containers::String demangle(ranges::StringView mangled) YACSL_API;

} // namespace yacsl::debug
