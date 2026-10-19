#pragma once

#include "yacsl/os/file.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::os::posix {

inline bool strip_file_open_flag(File::OpenFlags* flags,
                                 File::OpenFlags  flag) noexcept {
  File::OpenFlags const stripped = *flags & ~flag;
  if (stripped != *flags) {
    *flags = stripped;
    return true;
  }
  return false;
}

struct OpenedFile {
  alignas(16) File::Handle handle;
  usize cursor_pos;
};

int to_oflag(File::OpenFlags flags) noexcept;

} // namespace yacsl::os::posix
