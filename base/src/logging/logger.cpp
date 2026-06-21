module base.logging.logger;

import base.containers.stack_storage;

namespace base {

[[nodiscard]] Logger& Logger::get_default_instance() noexcept {
  static StackStorage<Logger> storage;
  static Logger&              reference = storage.construct();
  return reference;
}

} // namespace base
