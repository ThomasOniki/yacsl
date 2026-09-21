#ifdef _WIN32

#include "base/type_aliases.hpp"
#include "base/type_traits/limits.hpp"
#include "base/utils/isize.hpp"
#include "win32-utils.hpp"
#include <fileapi.h>
#include <intsafe.h>
#include <windows.h>

namespace base::os {

containers::Result<File, File::OpenError>
File::open(char const* path, File::Flags flags) noexcept {
  containers::Result<HANDLE, OpenError> handle = open_file_handle(path, flags);
  if (handle.is_err()) [[unlikely]] {
    return handle.assert_err();
  }
  File opened;
  opened.handle_ = handle.assert_ok();

  if (strip_file_open_flag(&flags, File::Flags::SeekEnd)) {
    LONG high = 0;
    opened.cursor_pos_ =
        static_cast<usize>(SetFilePointer(opened.handle_, 0, &high, FILE_END));
    opened.cursor_pos_ +=
        static_cast<usize>(high) * type_traits::Limits<LONG>::MAX;
    if (opened.cursor_pos_ == static_cast<usize>(INVALID_SET_FILE_POINTER)) {
      DWORD const err = GetLastError();
      if (err != NO_ERROR) {
        debug::panic("SetFilePointer failed: {}", win32_err_str(err));
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
usize File::write(this File& self, ranges::Slice<uchar const> data) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(WriteFile(self.handle_, data.data(),
                             static_cast<DWORD>(data.size()), &ret, nullptr));
  return static_cast<usize>(ret);
}
usize File::read(this File& self, ranges::Slice<uchar> buffer) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(ReadFile(self.handle_, buffer.data(),
                            static_cast<DWORD>(buffer.size()), &ret, nullptr));
  return static_cast<usize>(ret);
}

void File::seek_begin(this File& self, utils::isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / type_traits::Limits<LONG>::MAX);
  DWORD pos_low = SetFilePointer(self.handle_, low, &high, FILE_BEGIN);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      debug::panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  self.cursor_pos_ =
      static_cast<usize>(high) * type_traits::Limits<LONG>::MAX + pos_low;
}
void File::seek_end(this File& self, utils::isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / type_traits::Limits<LONG>::MAX);
  DWORD pos_low = SetFilePointer(self.handle_, low, &high, FILE_END);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      debug::panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  self.cursor_pos_ =
      static_cast<usize>(high) * type_traits::Limits<LONG>::MAX + pos_low;
}
void File::seek_cursor(this File& self, utils::isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / type_traits::Limits<LONG>::MAX);
  DWORD pos_low = SetFilePointer(self.handle_, low, &high, FILE_CURRENT);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      debug::panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  self.cursor_pos_ =
      static_cast<usize>(high) * type_traits::Limits<LONG>::MAX + pos_low;
}
// NOLINTEND(readability-make-member-function-const)

bool File::is_tty(this File const& self) noexcept {
  if (self.is_tty_ == IsTtyFlag::NotComputed) {
    self.is_tty_ =
        static_cast<IsTtyFlag>(GetFileType(self.handle_) == FILE_TYPE_CHAR);
  }
  return static_cast<bool>(self.is_tty_);
}

} // namespace base::os

#endif
