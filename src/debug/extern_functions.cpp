#include "yacsl/debug/_extern_functions.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/debug/stack_trace.hpp"

namespace yacsl::debug::internal {

void extern_log_error(ranges::StringView msg) { log_error(msg); }

void extern_panic(ranges::StringView msg, SourceLocation location) noexcept {
  panic_impl(fmt::format("panic at {}\n{}\nstacktrace: {}", location, msg,
                            StackTrace::current()));
}

} // namespace yacsl::debug::internal
