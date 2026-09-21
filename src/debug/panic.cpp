#include "base/containers/string.hpp"
#include "base/debug/log.hpp"
#include "base/debug/stack_trace.hpp"
#include <stdlib.h>

namespace base::debug {

[[noreturn]] void panic(containers::String const& formatted,
                        SourceLocation            location) noexcept {
  Logger::get_default_instance().log(Logger::Level::Panic, "panicked at {}",
                                     location);
  Logger::get_default_instance().log(Logger::Level::Panic, formatted);
  StackTrace::current().print(Logger::Level::Panic);
  Logger::get_default_instance().flush();
  abort();
}

} // namespace base::debug
