#include "yacsl/format/format_string_parser.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/ranges/string_view.hpp"

namespace yacsl::format {

void FormatStringParser::log_error_no_deps(ranges::StringView msg) noexcept {
  debug::Logger::get_default_instance().log(debug::Logger::Level::Error,
                                            containers::String(msg));
}

} // namespace yacsl::format
