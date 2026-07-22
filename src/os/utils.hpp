#pragma once

#ifdef _WIN32
#include <errhandlingapi.h>
#include <intsafe.h>
#include <windef.h>
#include <winnt.h>
#else
#include <errno.h>
#endif

#include "base/containers/result.hpp"
#include "base/os/common.hpp"
#include "base/os/file.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"

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

#ifdef _WIN32

StringView        win32_err_str(DWORD err) noexcept;
inline StringView win32_err_str() noexcept { return win32_err_str(GetLastError()); }
void              try_syscall(BOOL success, StringView function_name) noexcept;

#else

int               unix_flags(FileOpenFlags flags) noexcept;
StringView        errno_str(int err) noexcept;
inline StringView errno_str() noexcept { return errno_str(errno); }
void              try_syscall(bool success, StringView function_name) noexcept;

#endif

} // namespace base
