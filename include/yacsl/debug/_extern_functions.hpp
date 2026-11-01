#pragma once

#include "yacsl/compiler/export.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/ranges/string_view.hpp"

namespace yacsl::debug::detail {

extern void extern_log_error(ranges::StringView msg) YACSL_API;

[[noreturn]] extern void extern_panic(
    ranges::StringView msg,
    SourceLocation     location = SourceLocation::current()) noexcept YACSL_API;

} // namespace yacsl::debug::detail
