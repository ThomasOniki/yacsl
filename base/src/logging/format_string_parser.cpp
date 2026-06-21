module base.logging.format_string_parser;

import base.logging.common;
import base.logging.format_string;
import base.logging.logger;
import base.containers.char_buffer;
import base.utilities.type_aliases;

namespace base {

void FormatStringParser::log_error_no_deps(StringView msg) noexcept {
  CharBuffer buf;
  buf.append(LOG_LEVEL_STRINGS[static_cast<u8>(LogLevel::Error)]);
  buf.append(msg);
  buf.append('\n');
  Logger::get_default_instance().log(LogLevel::Error, buf);
}

} // namespace base
