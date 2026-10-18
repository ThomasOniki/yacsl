#pragma once

#ifdef _WIN32

#include "yacsl/containers/result.hpp"
#include "yacsl/os/file.hpp"
#include "yacsl/type_aliases.hpp"
#include <errhandlingapi.h>
#include <intsafe.h>
#include <windef.h>
#include <winnt.h>

namespace yacsl::os::windows {

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

// SeekEnd will be ignored
containers::Result<HANDLE, File::OpenError>
open_file_handle(char const* path, File::OpenFlags flags);

} // namespace yacsl::os::windows

#endif
