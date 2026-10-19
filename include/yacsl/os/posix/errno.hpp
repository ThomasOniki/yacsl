#pragma once

#include "yacsl/compiler/export.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/ranges/string_view.hpp"
#include <errno.h>

namespace yacsl::os::posix {

ranges::StringView        errno_str(int err) noexcept YACSL_API;
inline ranges::StringView errno_str() noexcept { return errno_str(errno); }

inline auto try_unix(auto                  return_value,
                     debug::SourceLocation location =
                         debug::SourceLocation::current()) noexcept {
  debug::assert_(return_value != -1, "Unix function failed at {}:\n{}",
                 location, errno_str());
  return return_value;
}

} // namespace yacsl::os::posix
