#pragma once

#include "base/ranges/string_view.hpp"

namespace base {

struct SourceLocation {
  static consteval SourceLocation
  current(u32 line = __builtin_LINE(), StringView file = __builtin_FILE(),
          StringView function = __builtin_FUNCTION()) noexcept {
    return SourceLocation{
        .line     = line,
        .file     = file,
        .function = function,
    };
  }

  u32        line = 0;
  StringView file;
  StringView function;
};

} // namespace base
