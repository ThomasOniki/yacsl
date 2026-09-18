#include "base/log/logger.hpp"
#include "base/containers/stack_storage.hpp"

namespace base {

[[nodiscard]] Logger& Logger::get_default_instance() noexcept {
  static StackStorage<Logger> storage;
  static Logger&              reference = storage.construct();
  return reference;
}

} // namespace base
