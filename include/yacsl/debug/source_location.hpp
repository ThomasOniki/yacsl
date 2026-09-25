#pragma once

#include "yacsl/ranges/string_view.hpp"

namespace yacsl::debug {

struct SourceLocation {
  static consteval SourceLocation
  current(u32                line     = __builtin_LINE(),
          ranges::StringView file     = __builtin_FILE(),
          ranges::StringView function = __builtin_FUNCTION()) noexcept {
    return SourceLocation{
        .line     = line,
        .file     = file,
        .function = function,
    };
  }

  u32                line = 0;
  ranges::StringView file;
  ranges::StringView function;
};

} // namespace yacsl::debug
