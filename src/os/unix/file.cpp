#ifndef _WIN32

#include "yacsl/os/file.hpp"
#include "utils.hpp"
#include "yacsl/containers/error.hpp"
#include "yacsl/containers/result.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/isize.hpp"
#include "yacsl/utils/nil.hpp"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

namespace yacsl::os {

containers::Result<File, File::OpenError>
File::open(char const* path, File::OpenFlags flags) noexcept {
  auto const result = open_file_handle(path, flags);
  if (result.is_error()) {
    return result.error_unchecked();
  }
  File opened;
  opened.handle_ = result.value_unchecked();
  if (strip_file_open_flag(&flags, File::OpenFlags::SeekEnd) &&
      (flags & File::OpenFlags::Truncate) == File::OpenFlags::None) {
    opened.cursor_pos_ = static_cast<usize>(
        YACSL_TRY_SYSCALL(::lseek(opened.handle_, 0, SEEK_END)));
  }
  return opened;
}

void File::close(this File& self) noexcept {
  if (self.handle_ != NIL_HANDLE) {
    YACSL_TRY_SYSCALL(::close(self.handle_));
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
usize File::write(this File& self, ranges::Slice<u8 const> data) noexcept {
  ssize_t const result =
      YACSL_TRY_SYSCALL(::write(self.handle_, data.data(), data.len()));
  return static_cast<usize>(result);
}
usize File::read(this File& self, ranges::Slice<u8> buffer) noexcept {
  ssize_t const result =
      YACSL_TRY_SYSCALL(::read(self.handle_, buffer.data(), buffer.len()));
  return static_cast<usize>(result);
}

void File::seek_begin(this File& self, utils::isize offset) noexcept {
  off_t const result = YACSL_TRY_SYSCALL(
      ::lseek(self.handle_, static_cast<off_t>(offset), SEEK_SET));
  self.cursor_pos_ = static_cast<usize>(result);
}
void File::seek_end(this File& self, utils::isize offset) noexcept {
  off_t const result = YACSL_TRY_SYSCALL(
      ::lseek(self.handle_, static_cast<off_t>(offset), SEEK_END));
  self.cursor_pos_ = static_cast<usize>(result);
}
void File::seek_cursor(this File& self, utils::isize offset) noexcept {
  off_t const result = YACSL_TRY_SYSCALL(
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

[[nodiscard]] containers::Error<File::OpenError>
File::access(char const* path, AccessFlags flags) {
  unsigned int mode = 0;
  if (static_cast<u8>(flags & AccessFlags::Read) > 0) {
    mode |= R_OK;
  }
  if (static_cast<u8>(flags & AccessFlags::Write) > 0) {
    mode |= W_OK;
  }
  if (static_cast<u8>(flags & AccessFlags::Execute) > 0) {
    mode |= X_OK;
  }
  if (::access(path, static_cast<int>(mode)) == 0) {
    return utils::NIL;
  }
  switch (errno) {
  case ENOENT:
    return OpenError::NotFound;
  case EACCES:
    return OpenError::PermissionDenied;
  default:
    debug::panic("access() failed: {}", errno_str());
  }
}

} // namespace yacsl::os

#endif
