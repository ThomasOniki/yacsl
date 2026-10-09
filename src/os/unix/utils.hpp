#pragma once

#ifndef _WIN32

#include "yacsl/debug/assert.hpp"
#include "yacsl/os/file.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"
#include <errno.h>

namespace yacsl::os {

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

int                       unix_flags(File::OpenFlags flags) noexcept;
ranges::StringView        errno_str(int err) noexcept;
inline ranges::StringView errno_str() noexcept { return errno_str(errno); }

// SeekEnd will be ignored
containers::Result<int, File::OpenError>
open_file_handle(char const* path, File::OpenFlags flags);

} // namespace yacsl::os

#define YACSL_TRY_SYSCALL(syscall)                                             \
  [&] {                                                                        \
    auto const _result = syscall;                                              \
    debug::assert_(_result != -1, #syscall " failed: {}", errno_str());        \
    return _result;                                                            \
  }()

#endif
