#include "yacsl/containers/string.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/debug/stack_trace.hpp"
#include <stdlib.h>

namespace yacsl::debug {

[[noreturn]] void panic(containers::String const& formatted,
                        SourceLocation            location) noexcept {
  Logger::get_default_instance().log(Logger::Level::Panic, "panicked at {}",
                                     location);
  Logger::get_default_instance().log(Logger::Level::Panic, formatted);
  StackTrace::current().print(Logger::Level::Panic);
  Logger::get_default_instance().flush();
  abort();
}

} // namespace yacsl::debug
