#include "base/logging/logger.hpp"
#include "base/containers/stack_storage.hpp"
#include "base/containers/string_view.hpp"

namespace base {

Logger& Logger::get_default_instance() noexcept {
  static StackStorage<Logger> storage;
  static Logger&              reference = storage.construct();
  return reference;
}

void log_trace3(StringView msg) { Logger::get_default_instance().trace3(msg); }
void log_trace2(StringView msg) { Logger::get_default_instance().trace2(msg); }
void log_trace1(StringView msg) { Logger::get_default_instance().trace1(msg); }
void log_debug(StringView msg) { Logger::get_default_instance().debug(msg); }
void log_info(StringView msg) { Logger::get_default_instance().info(msg); }
void log_notice(StringView msg) { Logger::get_default_instance().notice(msg); }
void log_warning(StringView msg) { Logger::get_default_instance().warn(msg); }
void log_error(StringView msg) { Logger::get_default_instance().error(msg); }

} // namespace base
