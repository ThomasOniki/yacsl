module;

#ifndef _WIN32
#include "try_syscall.hpp"
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#endif

module base.sys.file;

import base.sys.utils;
import base.containers.result;
import base.utilities.nil;
import base.logging.panic;

#ifndef _WIN32

namespace base {

Error<File::OpenError> File::open(const char* path, FileOpenFlags flags) noexcept {
  close();
  const Result<FileHandle, OpenError> result = open_file(path, flags);
  if (result.is_err()) [[unlikely]] {
    return result.assert_err();
  }
  handle_ = result.assert_ok();
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
usize File::write(BasicStringView<uchar> data) noexcept {
  const isize result = ::write(handle_, data.data(), data.size());
  panic_if(result == -1, "::write() failed: {}", errno_str());
  return static_cast<usize>(result);
}
usize File::read(uchar* buf, usize size) noexcept {
  const isize result = ::read(handle_, buf, size);
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

} // namespace base

#endif
