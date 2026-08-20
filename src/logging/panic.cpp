#include "base/containers/string.hpp"
#include "base/logging/logger.hpp"
#include "base/os/stack_trace.hpp"
#include <stdlib.h>

namespace base {

[[noreturn]] void panic(const String& formatted) noexcept {
  Logger::get_default_instance().log(Logger::Level::Panic, formatted);
  StackTrace::current().print(Logger::Level::Panic);
  Logger::get_default_instance().flush();
  abort();
}

} // namespace base
