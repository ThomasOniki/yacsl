#include "base/containers/stack_storage.hpp"
#include "base/debug/log.hpp"

namespace base {

[[nodiscard]] Logger& Logger::get_default_instance() noexcept {
  static StackStorage<Logger> storage;
  static Logger&              reference = storage.construct();
  return reference;
}

} // namespace base
