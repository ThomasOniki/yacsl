#include "base/logging/logger.hpp"
#include "base/containers/string_view.hpp"
#include "base/threads/mutex.hpp"

#include <stdio.h>

namespace base {

static Mutex& get_stdout_mutex() {
  static Mutex instance;
  return instance;
}

Logger& Logger::get_default_instance() {
  static Logger instance;
  return instance;
}
void Logger::write_to_stdout(const StringView& str) {
  const MutexLock lock(get_stdout_mutex());
  (void)fwrite(str.data(), str.size(), 1, stdout);
}
void Logger::write_to_stderr(const StringView& str) {
  static Mutex    mtx;
  const MutexLock lock(mtx);
  (void)fwrite(str.data(), str.size(), 1, stderr);
}
void Logger::flush() {
  const MutexLock lock(get_stdout_mutex());
  (void)fflush(stdout);
}

void log_trace3(const StringView& msg) { Logger::get_default_instance().trace3(msg); }
void log_trace2(const StringView& msg) { Logger::get_default_instance().trace2(msg); }
void log_trace1(const StringView& msg) { Logger::get_default_instance().trace1(msg); }
void log_debug(const StringView& msg) { Logger::get_default_instance().debug(msg); }
void log_info(const StringView& msg) { Logger::get_default_instance().info(msg); }
void log_notice(const StringView& msg) { Logger::get_default_instance().notice(msg); }
void log_warning(const StringView& msg) { Logger::get_default_instance().warn(msg); }
void log_error(const StringView& msg) { Logger::get_default_instance().error(msg); }
void log_critical(const StringView& msg) { Logger::get_default_instance().critical(msg); }

} // namespace base
