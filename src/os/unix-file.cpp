#ifndef _WIN32

#include "base/containers/error.hpp"
#include "base/containers/result.hpp"
#include "base/os/file.hpp"
#include "base/ranges/slice.hpp"
#include "base/utilities/isize.hpp"
#include "base/utilities/nil.hpp"
#include "base/utilities/type_aliases.hpp"
#include "unix-utils.hpp"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

namespace base {

Error<File::OpenError> File::open(this File& self, char const* path, File::Flags flags) noexcept {
  self.close();
  Result<OpenedFile, OpenError> const result = open_file(path, flags);
  if (result.is_err()) [[unlikely]] {
    return result.assert_err();
  }
  self.handle_     = result.assert_ok().handle;
  self.cursor_pos_ = result.assert_ok().cursor_pos;
  return NIL;
}

void File::close(this File& self) noexcept {
  if (self.is_open()) {
    BASE_TRY_SYSCALL(::close(self.handle_));
    self.handle_ = NIL_HANDLE;
  }
}

void File::open_stdin(this File& self) noexcept { self.handle_ = STDIN_FILENO; }
void File::open_stdout(this File& self) noexcept { self.handle_ = STDOUT_FILENO; }
void File::open_stderr(this File& self) noexcept { self.handle_ = STDERR_FILENO; }

// NOLINTBEGIN(readability-make-member-function-const)
usize File::write(this File& self, Slice<uchar const> data) noexcept {
  ssize_t const result = BASE_TRY_SYSCALL(::write(self.handle_, data.data(), data.size()));
  return static_cast<usize>(result);
}
usize File::read(this File& self, Slice<uchar> buffer) noexcept {
  ssize_t const result = BASE_TRY_SYSCALL(::read(self.handle_, buffer.data(), buffer.size()));
  return static_cast<usize>(result);
}

void File::seek_begin(this File& self, isize offset) noexcept {
  off_t const result =
      BASE_TRY_SYSCALL(::lseek(self.handle_, static_cast<off_t>(offset), SEEK_SET));
  self.cursor_pos_ = static_cast<usize>(result);
}
void File::seek_end(this File& self, isize offset) noexcept {
  off_t const result =
      BASE_TRY_SYSCALL(::lseek(self.handle_, static_cast<off_t>(offset), SEEK_END));
  self.cursor_pos_ = static_cast<usize>(result);
}
void File::seek_cursor(this File& self, isize offset) noexcept {
  off_t const result =
      BASE_TRY_SYSCALL(::lseek(self.handle_, static_cast<off_t>(offset), SEEK_CUR));
  self.cursor_pos_ = static_cast<usize>(result);
}
// NOLINTEND(readability-make-member-function-const)

bool File::is_tty(this File const& self) noexcept {
  if (self.is_tty_ == IsTtyFlag::NotComputed) {
    self.is_tty_ = static_cast<IsTtyFlag>(isatty(self.handle_));
  }
  return static_cast<bool>(self.is_tty_);
}

} // namespace base

#endif
