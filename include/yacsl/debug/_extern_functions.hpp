#pragma once

#include "yacsl/debug/source_location.hpp"
#include "yacsl/ranges/string_view.hpp"

namespace yacsl::debug::internal {

extern void extern_log_error(ranges::StringView msg);

[[noreturn]] extern void
extern_panic(ranges::StringView msg,
             SourceLocation     location = SourceLocation::current()) noexcept;

} // namespace yacsl::debug::internal
