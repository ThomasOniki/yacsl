#ifndef _WIN32
#include "base/containers/error.hpp"
#include "base/containers/result.hpp"
#include "base/logging/panic.hpp"
#include "base/os/common.hpp"
#include "base/os/file.hpp"
#include "base/ranges/slice.hpp"
#include "base/utilities/isize.hpp"
#include "base/utilities/nil.hpp"
#include "base/utilities/type_aliases.hpp"
#include "try_syscall.hpp"
#include "unix-utils.hpp"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

namespace base {

Error<File::OpenError> File::open(const char* path, FileOpenFlags flags) noexcept {
  close();
  const Result<OpenedFile, OpenError> result = open_file(path, flags);
  if (result.is_err()) [[unlikely]] {
    return result.assert_err();
  }
  handle_     = result.assert_ok().handle;
  cursor_pos_ = result.assert_ok().cursor_pos;
  return NIL;
}

void File::close() noexcept {
  if (is_open()) {
    BASE_TRY_SYSCALL(::close(handle_));
    handle_ = NIL_FILE_HANDLE;
  }
}

void File::open_stdin() noexcept { handle_ = STDIN_FILENO; }
void File::open_stdout() noexcept { handle_ = STDOUT_FILENO; }
void File::open_stderr() noexcept { handle_ = STDERR_FILENO; }

// NOLINTBEGIN(readability-make-member-function-const)
usize File::write(Slice<const uchar> data) noexcept {
  const isize result = ::write(handle_, data.data(), data.size());
  panic_if(result == -1, "::write() failed: {}", errno_str());
  return static_cast<usize>(result);
}
usize File::read(Slice<uchar> buffer) noexcept {
  const isize result = ::read(handle_, buffer.data(), buffer.size());
  panic_if(result == -1, "::read() failed: {}", errno_str());
  return static_cast<usize>(result);
}

void File::seek_begin(isize offset) noexcept {
  const off_t result = ::lseek(handle_, static_cast<off_t>(offset), SEEK_SET);
  panic_if(result == -1, "::lseek() failed: {}", errno_str());
  cursor_pos_ = static_cast<usize>(result);
}
void File::seek_end(isize offset) noexcept {
  const off_t result = ::lseek(handle_, static_cast<off_t>(offset), SEEK_END);
  panic_if(result == -1, "::lseek() failed: {}", errno_str());
  cursor_pos_ = static_cast<usize>(result);
}
void File::seek_cursor(isize offset) noexcept {
  const off_t result = ::lseek(handle_, static_cast<off_t>(offset), SEEK_CUR);
  panic_if(result == -1, "::lseek() failed: {}", errno_str());
  cursor_pos_ = static_cast<usize>(result);
}
// NOLINTEND(readability-make-member-function-const)

bool File::is_tty() const noexcept {
  if (is_tty_ == IsTtyFlag::NotComputed) {
    is_tty_ = static_cast<IsTtyFlag>(isatty(handle_));
  }
  return static_cast<bool>(is_tty_);
}

} // namespace base

#endif
