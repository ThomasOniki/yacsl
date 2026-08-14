#pragma once

#ifndef _WIN32

#include "base/containers/result.hpp"
#include "base/os/file.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"
#include <errno.h>

namespace base {

inline bool strip_file_open_flag(File::Flags* flags, File::Flags flag) noexcept {
  const File::Flags stripped = *flags & ~flag;
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
Result<OpenedFile, File::OpenError> open_file(const char* path, File::Flags flags) noexcept;

int               unix_flags(File::Flags flags) noexcept;
StringView        errno_str(int err) noexcept;
inline StringView errno_str() noexcept { return errno_str(errno); }
} // namespace base

#define BASE_TRY_SYSCALL(syscall)                                                                  \
  [&]() {                                                                                          \
    auto result = syscall;                                                                         \
    panic_if(result == -1, #syscall " failed: {}", errno_str());                                   \
    return result;                                                                                 \
  }()

#endif
