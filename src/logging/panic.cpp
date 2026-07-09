module;

#include <stdlib.h>

module base.logging.panic;

import base.logging.logger;
import base.utilities.stack_trace;

namespace base {

[[noreturn]] void panic(const String& formatted) noexcept {
  Logger::get_default_instance().log(LogLevel::Panic, formatted);
  StackTrace::current().print(LogLevel::Panic);
  Logger::get_default_instance().flush();
  abort();
}

} // namespace base
