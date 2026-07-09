module base.formatting.format_string_parser;

import base.logging.common;
import base.formatting.format_string;
import base.logging.logger;
import base.containers.string;
import base.utilities.type_aliases;

namespace base {

void FormatStringParser::log_error_no_deps(StringView msg) noexcept {
  Logger::get_default_instance().log(LogLevel::Error, String(msg));
}

} // namespace base
