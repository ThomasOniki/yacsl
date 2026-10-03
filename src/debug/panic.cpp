#include "yacsl/containers/string.hpp"
#include "yacsl/debug/log.hpp"
#include <stdlib.h>

namespace yacsl::debug {

[[noreturn]] void panic_impl(containers::String const& formatted) noexcept {
  log(Logger::Level::Panic, formatted);
  flush_logger();
  abort();
}

} // namespace yacsl::debug
