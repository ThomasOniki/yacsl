#include "base/containers/string.hpp"
#include "base/logging/common.hpp"
#include "base/logging/logger.hpp"
#include "base/utilities/stack_trace.hpp"
#include <stdlib.h>

namespace base {

[[noreturn]] void panic(const String& formatted) noexcept {
  Logger::get_default_instance().log(LogLevel::Panic, formatted);
  StackTrace::current().print(LogLevel::Panic);
  Logger::get_default_instance().flush();
  abort();
}

} // namespace base
