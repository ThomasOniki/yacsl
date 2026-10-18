#ifdef _WIN32

#include "yacsl/os/windows/handle.hpp"
#include "yacsl/containers/result.hpp"
#include "yacsl/os/file.hpp"
#include "yacsl/os/windows/win_err.hpp"

namespace yacsl::os::windows {

containers::Result<HANDLE, File::OpenError>
open_file_handle(char const* path, File::OpenFlags flags) {
  DWORD access = 0;
  if (strip_file_open_flag(&flags, File::OpenFlags::Read)) {
    access |= GENERIC_READ;
  }
  if (strip_file_open_flag(&flags, File::OpenFlags::Write)) {
    access |= GENERIC_WRITE;
  }
  SECURITY_ATTRIBUTES security_attr = {
      .nLength              = sizeof(SECURITY_ATTRIBUTES),
      .lpSecurityDescriptor = nullptr,
      .bInheritHandle =
          strip_file_open_flag(&flags, File::OpenFlags::Inheritable) ? TRUE
                                                                     : FALSE,
  };

  bool const trunc  = strip_file_open_flag(&flags, File::OpenFlags::Truncate);
  bool const create = strip_file_open_flag(&flags, File::OpenFlags::Create);
  bool const excl   = strip_file_open_flag(&flags, File::OpenFlags::Exclusive);
  DWORD      creation_disposition = 0;

  if (create) {
    if (excl) {
      creation_disposition = CREATE_NEW;
    } else if (trunc) {
      creation_disposition = CREATE_ALWAYS;
    } else {
      creation_disposition = OPEN_ALWAYS;
    }
  } else {
    if (trunc) {
      creation_disposition = TRUNCATE_EXISTING;
    } else {
      creation_disposition = OPEN_EXISTING;
    }
  }

  HANDLE handle = CreateFileA(
      path, access, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
      &security_attr, creation_disposition, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (handle == INVALID_HANDLE_VALUE) {
    DWORD const err = GetLastError();
    switch (err) {
    case ERROR_FILE_NOT_FOUND:
      return File::OpenError::NotFound;
    case ERROR_ALREADY_EXISTS:
      return File::OpenError::AlreadyExists;
    case ERROR_ACCESS_DENIED:
      return File::OpenError::PermissionDenied;
    default:
      debug::panic("CreateFileA failed: {}", win_err_str(err));
    }
  }

  return handle;
}

} // namespace yacsl::os::windows

#endif
