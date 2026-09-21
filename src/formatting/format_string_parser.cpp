#include "base/format/format_string_parser.hpp"
#include "base/containers/string.hpp"
#include "base/debug/log.hpp"
#include "base/ranges/string_view.hpp"

namespace base::format {

void FormatStringParser::log_error_no_deps(ranges::StringView msg) noexcept {
  debug::Logger::get_default_instance().log(debug::Logger::Level::Error,
                                            containers::String(msg));
}

} // namespace base::format
