#include "base/containers/stack_storage.hpp"
#include "base/debug/log.hpp"

namespace base::debug {

[[nodiscard]] Logger& Logger::get_default_instance() noexcept {
  static containers::StackStorage<Logger> storage;
  static Logger&                          reference = storage.construct();
  return reference;
}

} // namespace base::debug
