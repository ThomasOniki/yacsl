#ifdef _WIN32

#include "base/type_traits/limits.hpp"
#include "base/utilities/isize.hpp"
#include "base/utilities/type_aliases.hpp"
#include "win32-utils.hpp"
#include <fileapi.h>
#include <intsafe.h>
#include <windows.h>

namespace base {

[[nodiscard]] Error<File::OpenError> File::open(this File& self, char const* path,
                                                File::Flags flags) noexcept {
  self.close();
  Result<OpenedFile, OpenError> const result = open_file(path, flags);
  if (result.is_ok()) [[likely]] {
    self.handle_     = result.assert_ok().handle;
    self.cursor_pos_ = result.assert_ok().cursor_pos;
    return NIL;
  }
  self.handle_ = NIL_HANDLE;
  return result.assert_err();
}

void File::close(this File& self) noexcept {
  if (self.handle_ != NIL_HANDLE) {
    BASE_TRY_SYSCALL(CloseHandle(self.handle_));
  }
}

void File::open_stdin(this File& self) noexcept { self.handle_ = GetStdHandle(STD_INPUT_HANDLE); }
void File::open_stdout(this File& self) noexcept { self.handle_ = GetStdHandle(STD_OUTPUT_HANDLE); }
void File::open_stderr(this File& self) noexcept { self.handle_ = GetStdHandle(STD_ERROR_HANDLE); }

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
