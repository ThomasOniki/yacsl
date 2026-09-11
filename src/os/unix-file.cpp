#ifndef _WIN32

#include "base/containers/result.hpp"
#include "base/logging/panic.hpp"
#include "base/os/file.hpp"
#include "base/ranges/slice.hpp"
#include "base/utilities/isize.hpp"
#include "base/utilities/type_aliases.hpp"
#include "unix-utils.hpp"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

namespace base {

Result<File, File::OpenError> File::open(char const* path,
                                         File::Flags flags) noexcept {

  bool const seek_end = strip_file_open_flag(&flags, File::Flags::SeekEnd) &&
                        (flags & File::Flags::Trunc) == File::Flags::None;
  File       opened;
  opened.handle_ = ::open(path, unix_flags(flags), 0644);
  if (opened.handle_ == -1) [[unlikely]] {
    switch (errno) {
    case ENOENT:
      return OpenError::FileNotFound;
    default:
      panic("::open() failed: {}", errno_str());
    }
  }
  if (seek_end) {
    opened.cursor_pos_ = static_cast<usize>(
        BASE_TRY_SYSCALL(::lseek(opened.handle_, 0, SEEK_END)));
  }
  return opened;
}

void File::close(this File& self) noexcept {
  if (self.handle_ != NIL_HANDLE) {
    BASE_TRY_SYSCALL(::close(self.handle_));
    self.handle_ = NIL_HANDLE;
  }
}

File File::open_stdin() noexcept {
  File opened;
  opened.handle_ = STDIN_FILENO;
  return opened;
}
File File::open_stdout() noexcept {
  File opened;
  opened.handle_ = STDOUT_FILENO;
  return opened;
}
File File::open_stderr() noexcept {
  File opened;
  opened.handle_ = STDERR_FILENO;
  return opened;
}

// NOLINTBEGIN(readability-make-member-function-const)
usize File::write(this File& self, Slice<uchar const> data) noexcept {
  ssize_t const result =
      BASE_TRY_SYSCALL(::write(self.handle_, data.data(), data.size()));
  return static_cast<usize>(result);
}
usize File::read(this File& self, Slice<uchar> buffer) noexcept {
  ssize_t const result =
      BASE_TRY_SYSCALL(::read(self.handle_, buffer.data(), buffer.size()));
  return static_cast<usize>(result);
}

void File::seek_begin(this File& self, isize offset) noexcept {
  off_t const result = BASE_TRY_SYSCALL(
      ::lseek(self.handle_, static_cast<off_t>(offset), SEEK_SET));
  self.cursor_pos_ = static_cast<usize>(result);
}
void File::seek_end(this File& self, isize offset) noexcept {
  off_t const result = BASE_TRY_SYSCALL(
      ::lseek(self.handle_, static_cast<off_t>(offset), SEEK_END));
  self.cursor_pos_ = static_cast<usize>(result);
}
void File::seek_cursor(this File& self, isize offset) noexcept {
  off_t const result = BASE_TRY_SYSCALL(
      ::lseek(self.handle_, static_cast<off_t>(offset), SEEK_CUR));
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
