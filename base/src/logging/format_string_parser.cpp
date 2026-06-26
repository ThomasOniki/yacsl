module base.logging.format_string_parser;

import base.logging.common;
import base.logging.format_string;
import base.logging.logger;
import base.containers.string;
import base.utilities.type_aliases;

namespace base {

void FormatStringParser::log_error_no_deps(StringView msg) noexcept {
  String buf;
  buf += LOG_LEVEL_STRINGS[static_cast<u8>(LogLevel::Error)];
  buf += msg;
  buf += '\n';
  Logger::get_default_instance().log(LogLevel::Error, buf);
}

} // namespace base
