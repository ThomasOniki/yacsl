module;

#include <stdlib.h>

module base.logging.panic;

import base.logging.logger;

namespace base {

[[noreturn]] void panic(const String& formatted) noexcept {
  Logger::get_default_instance().log(LogLevel::Panic, formatted);
  abort();
}

} // namespace base
