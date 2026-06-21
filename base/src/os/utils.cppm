module;

#ifdef _WIN32
#include <errhandlingapi.h>
#include <intsafe.h>
#include <windef.h>
#include <winnt.h>
#else
#include <errno.h>
#endif

export module base.os.utils;

import base.os.common;
import base.os.file;
import base.ranges.string_view;
import base.type_traits.range_like;
import base.containers.result;

export namespace base {

bool strip_file_open_flag(FileOpenFlags* flags, FileOpenFlags flag) noexcept {
  const FileOpenFlags stripped = *flags & ~flag;
  if (stripped != *flags) {
    *flags = stripped;
    return true;
  }
  return false;
}

Result<FileHandle, File::OpenError> open_file(const char* path, FileOpenFlags flags) noexcept;

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
