#ifndef BASE_PLATFORM_WINDOWS

#include "base/containers/cstring.hpp"
#include "base/containers/error.hpp"
#include "base/containers/result.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/panic.hpp"
#include "base/sys/common.hpp"
#include "base/sys/file.hpp"
#include "base/utilities/nil.hpp"
#include "base/utilities/type_aliases.hpp"
#include "unix-utils.hpp"
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

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
    panic_if(::close(handle_) == -1, "::close() failed: {}", errno_str());
    handle_ = NIL_FILE_HANDLE;
  }
}

void File::open_stdin() noexcept { handle_ = STDIN_FILENO; }
void File::open_stdout() noexcept { handle_ = STDOUT_FILENO; }
void File::open_stderr() noexcept { handle_ = STDERR_FILENO; }

// NOLINTBEGIN(readability-make-member-function-const)
usize File::write(const BasicStringView<uchar>& data) noexcept {
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

#ifdef BASE_LSP_ONLY
template class base::BasicCString<char, 8>;
template base::BasicCString<char, 8>::BasicCString(const BasicStringView<char>& str);
#endif

#endif
