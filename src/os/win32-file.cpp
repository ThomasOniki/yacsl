#ifdef _WIN32

#include "base/type_traits/limits.hpp"
#include "base/utilities/isize.hpp"
#include "base/utilities/type_aliases.hpp"
#include "win32-utils.hpp"
#include <fileapi.h>
#include <intsafe.h>
#include <windows.h>

namespace base {

Result<File, File::OpenError> File::open(char const* path, File::Flags flags) noexcept {
  DWORD access = 0;
  if (strip_file_open_flag(&flags, File::Flags::Read)) {
    access |= GENERIC_READ;
  }
  if (strip_file_open_flag(&flags, File::Flags::Write)) {
    access |= GENERIC_WRITE;
  }
  SECURITY_ATTRIBUTES security_attr = {
      .nLength              = sizeof(SECURITY_ATTRIBUTES),
      .lpSecurityDescriptor = nullptr,
      .bInheritHandle = strip_file_open_flag(&flags, File::Flags::DontCloseOnExec) ? TRUE : FALSE,
  };

  bool const trunc                = strip_file_open_flag(&flags, File::Flags::Trunc);
  bool const create               = strip_file_open_flag(&flags, File::Flags::Create);
  DWORD      creation_disposition = 0;

  if (create) {
    if (trunc) {
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

  File opened;
  opened.handle_ =
      CreateFileA(path, access, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                  &security_attr, creation_disposition, FILE_ATTRIBUTE_NORMAL, nullptr);

  //  log_info("opened {} at {}.", path, handle);
  //  log_info("inheritable: {}", security_attr.bInheritHandle);

  if (opened.handle_ == INVALID_HANDLE_VALUE) {
    DWORD const err = GetLastError();
    switch (err) {
    case ERROR_FILE_NOT_FOUND:
      return File::OpenError::FileNotFound;
    default:
      panic("CreateFileA failed: {}", win32_err_str(err));
    }
  }

  if (strip_file_open_flag(&flags, File::Flags::SeekEnd)) {
    LONG high          = 0;
    opened.cursor_pos_ = static_cast<usize>(SetFilePointer(opened.handle_, 0, &high, FILE_END));
    opened.cursor_pos_ += static_cast<usize>(high) * Limits<LONG>::MAX;
    if (opened.cursor_pos_ == static_cast<usize>(INVALID_SET_FILE_POINTER)) {
      DWORD const err = GetLastError();
      if (err != NO_ERROR) {
        panic("SetFilePointer failed: {}", win32_err_str(err));
      }
    }
  }

  return opened;
}

void File::close(this File& self) noexcept {
  if (self.handle_ != NIL_HANDLE) {
    BASE_TRY_SYSCALL(CloseHandle(self.handle_));
  }
}

File File::open_stdin() noexcept {
  File opened;
  opened.handle_ = GetStdHandle(STD_INPUT_HANDLE);
  return opened;
}
File File::open_stdout() noexcept {
  File opened;
  opened.handle_ = GetStdHandle(STD_OUTPUT_HANDLE);
  return opened;
}
File File::open_stderr() noexcept {
  File opened;
  opened.handle_ = GetStdHandle(STD_ERROR_HANDLE);
  return opened;
}

// NOLINTBEGIN(readability-make-member-function-const)
usize File::write(this File& self, Slice<uchar const> data) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(
      WriteFile(self.handle_, data.data(), static_cast<DWORD>(data.size()), &ret, nullptr));
  return static_cast<usize>(ret);
}
usize File::read(this File& self, Slice<uchar> buffer) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(
      ReadFile(self.handle_, buffer.data(), static_cast<DWORD>(buffer.size()), &ret, nullptr));
  return static_cast<usize>(ret);
}

void File::seek_begin(this File& self, isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / Limits<LONG>::MAX);
  DWORD pos_low = SetFilePointer(self.handle_, low, &high, FILE_BEGIN);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  self.cursor_pos_ = static_cast<usize>(high) * Limits<LONG>::MAX + pos_low;
}
void File::seek_end(this File& self, isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / Limits<LONG>::MAX);
  DWORD pos_low = SetFilePointer(self.handle_, low, &high, FILE_END);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  self.cursor_pos_ = static_cast<usize>(high) * Limits<LONG>::MAX + pos_low;
}
void File::seek_cursor(this File& self, isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / Limits<LONG>::MAX);
  DWORD pos_low = SetFilePointer(self.handle_, low, &high, FILE_CURRENT);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  self.cursor_pos_ = static_cast<usize>(high) * Limits<LONG>::MAX + pos_low;
}
// NOLINTEND(readability-make-member-function-const)

bool File::is_tty(this File const& self) noexcept {
  if (self.is_tty_ == IsTtyFlag::NotComputed) {
    self.is_tty_ = static_cast<IsTtyFlag>(GetFileType(self.handle_) == FILE_TYPE_CHAR);
  }
  return static_cast<bool>(self.is_tty_);
}

} // namespace base

#endif
