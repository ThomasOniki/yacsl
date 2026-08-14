#pragma once

#ifndef _WIN32

#include "base/containers/result.hpp"
#include "base/os/common.hpp"
#include "base/os/file.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"
#include <errno.h>

namespace base {

inline bool strip_file_open_flag(FileOpenFlags* flags, FileOpenFlags flag) noexcept {
  const FileOpenFlags stripped = *flags & ~flag;
  if (stripped != *flags) {
    *flags = stripped;
    return true;
  }
  return false;
}

struct OpenedFile {
  alignas(16) FileHandle handle;
  usize cursor_pos;
};
Result<OpenedFile, File::OpenError> open_file(const char* path, FileOpenFlags flags) noexcept;

int               unix_flags(FileOpenFlags flags) noexcept;
StringView        errno_str(int err) noexcept;
inline StringView errno_str() noexcept { return errno_str(errno); }
} // namespace base

#endif
