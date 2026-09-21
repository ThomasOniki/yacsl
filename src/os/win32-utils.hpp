#pragma once

#ifdef _WIN32

#include "base/containers/result.hpp"
#include "base/os/file.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_aliases.hpp"
#include <errhandlingapi.h>
#include <intsafe.h>
#include <windef.h>
#include <winnt.h>

namespace base::os {

inline bool strip_file_open_flag(File::Flags* flags,
                                 File::Flags  flag) noexcept {
  File::Flags const stripped = *flags & ~flag;
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

ranges::StringView        win32_err_str(DWORD err) noexcept;
inline ranges::StringView win32_err_str() noexcept {
  return win32_err_str(GetLastError());
}

// SeekEnd will be ignored
containers::Result<HANDLE, File::OpenError> open_file_handle(char const* path,
                                                 File::Flags flags);

} // namespace base::os

#define BASE_TRY_SYSCALL(syscall)                                              \
  if ((syscall) == FALSE) [[unlikely]]                                         \
  debug::panic(#syscall " failed: {}", win32_err_str())

#endif
