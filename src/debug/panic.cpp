#include "yacsl/containers/string.hpp"
#include "yacsl/debug/log.hpp"
#include <stdlib.h>

namespace yacsl::debug {

[[noreturn]] void panic_impl(containers::String const& formatted) noexcept {
  Logger::get_default_instance().log(Logger::Level::Panic, formatted);
  Logger::get_default_instance().flush();
  abort();
}

} // namespace yacsl::debug
