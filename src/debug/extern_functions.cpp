#include "yacsl/debug/_extern_functions.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/debug/source_location.hpp"

namespace yacsl::debug::internal {

void extern_log_error(ranges::StringView msg) { log_error(msg); }

void extern_panic(ranges::StringView msg, SourceLocation location) noexcept {
  panic(msg, location);
}

} // namespace yacsl::debug::internal
