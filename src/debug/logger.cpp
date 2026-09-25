#include "yacsl/containers/stack_storage.hpp"
#include "yacsl/debug/log.hpp"

namespace yacsl::debug {

[[nodiscard]] Logger& Logger::get_default_instance() noexcept {
  static containers::StackStorage<Logger> storage;
  static Logger&                          reference = storage.construct();
  return reference;
}

} // namespace yacsl::debug
