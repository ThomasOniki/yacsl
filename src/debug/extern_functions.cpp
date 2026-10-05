#include "yacsl/debug/_extern_functions.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/ranges/string_view.hpp"

namespace yacsl::debug::internal {

void extern_log_error(ranges::StringView msg) { log_error(msg); }

void extern_panic(ranges::StringView msg, SourceLocation location) noexcept {
  panic_impl(fmt::format("panic at {}\n"
                         "{}\n"
                         "stacktrace:\n{}",
                         location, msg, StackTrace::current()));
}

} // namespace yacsl::debug::internal
