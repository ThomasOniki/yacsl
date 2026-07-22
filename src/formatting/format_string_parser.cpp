#include "base/formatting/format_string_parser.hpp"
#include "base/containers/string.hpp"
#include "base/logging/common.hpp"
#include "base/logging/logger.hpp"
#include "base/ranges/string_view.hpp"

namespace base {

void FormatStringParser::log_error_no_deps(StringView msg) noexcept {
  Logger::get_default_instance().log(LogLevel::Error, String(msg));
}

} // namespace base
