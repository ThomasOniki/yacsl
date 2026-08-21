#ifdef _WIN32

#include "base/type_traits/limits.hpp"
#include "base/utilities/isize.hpp"
#include "base/utilities/type_aliases.hpp"
#include "win32-utils.hpp"
#include <fileapi.h>
#include <intsafe.h>
#include <windows.h>

namespace base {

[[nodiscard]] Error<File::OpenError> File::open(char const* path, File::Flags flags) noexcept {
  close();
  Result<OpenedFile, OpenError> const result = open_file(path, flags);
  if (result.is_ok()) [[likely]] {
    handle_     = result.assert_ok().handle;
    cursor_pos_ = result.assert_ok().cursor_pos;
    return NIL;
  }
  handle_ = NIL_HANDLE;
  return result.assert_err();
}

void File::close() noexcept {
  if (handle_ != NIL_HANDLE) {
    BASE_TRY_SYSCALL(CloseHandle(handle_));
  }
}

void File::open_stdin() noexcept { handle_ = GetStdHandle(STD_INPUT_HANDLE); }
void File::open_stdout() noexcept { handle_ = GetStdHandle(STD_OUTPUT_HANDLE); }
void File::open_stderr() noexcept { handle_ = GetStdHandle(STD_ERROR_HANDLE); }

// NOLINTBEGIN(readability-make-member-function-const)
usize File::write(Slice<uchar const> data) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(WriteFile(handle_, data.data(), static_cast<DWORD>(data.size()), &ret, nullptr));
  return static_cast<usize>(ret);
}
usize File::read(Slice<uchar> buffer) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(
      ReadFile(handle_, buffer.data(), static_cast<DWORD>(buffer.size()), &ret, nullptr));
  return static_cast<usize>(ret);
}

void File::seek_begin(isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / Limits<LONG>::max);
  DWORD pos_low = SetFilePointer(handle_, low, &high, FILE_BEGIN);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  cursor_pos_ = static_cast<usize>(high) * Limits<LONG>::max + pos_low;
}
void File::seek_end(isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / Limits<LONG>::max);
  DWORD pos_low = SetFilePointer(handle_, low, &high, FILE_END);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  cursor_pos_ = static_cast<usize>(high) * Limits<LONG>::max + pos_low;
}
void File::seek_cursor(isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / Limits<LONG>::max);
  DWORD pos_low = SetFilePointer(handle_, low, &high, FILE_CURRENT);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  cursor_pos_ = static_cast<usize>(high) * Limits<LONG>::max + pos_low;
}
// NOLINTEND(readability-make-member-function-const)

bool File::is_tty() const noexcept {
  if (is_tty_ == IsTtyFlag::NotComputed) {
    is_tty_ = static_cast<IsTtyFlag>(GetFileType(handle_) == FILE_TYPE_CHAR);
  }
  return static_cast<bool>(is_tty_);
}

} // namespace base

#endif
