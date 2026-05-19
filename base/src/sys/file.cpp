#include "base/sys/file.hpp"
#include "base/containers/cstring.hpp"
#include "base/containers/error.hpp"
#include "base/containers/string_view.hpp"
#include "base/containers/unsigned_result.hpp"
#include "base/logging/errors.hpp"
#include "base/sys/common.hpp"
#include "base/sys/errno.hpp"
#include "base/utilities/nil.hpp"
#include "base/utilities/type_aliases.hpp"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

namespace base {

namespace internal {

static bool strip_flag(FileOpenFlags* flags, FileOpenFlags flag) noexcept {
  const FileOpenFlags stripped = *flags & ~flag;
  if (stripped != *flags) {
    *flags = stripped;
    return true;
  }
  return false;
}

static int unix_flags(FileOpenFlags flags) noexcept {
  unsigned int ret = 0;

  if (strip_flag(&flags, FileOpenFlags::Trunc)) {
    ret |= O_TRUNC;
  }
  if (strip_flag(&flags, FileOpenFlags::Create)) {
    ret |= O_CREAT;
  }
  if (strip_flag(&flags, FileOpenFlags::Append)) {
    ret |= O_APPEND;
  }
  if (!strip_flag(&flags, FileOpenFlags::DontCloseOnExec)) {
    ret |= O_CLOEXEC;
  }
  if (flags == FileOpenFlags::Read) {
    ret |= O_RDONLY;
  } else if (flags == FileOpenFlags::Write) {
    ret |= O_WRONLY;
  } else {
    ret |= O_RDWR;
  }
  return static_cast<int>(ret);
}

static UnsignedResult<unsigned int, File::OpenError> open_file(const char*   cstr_path,
                                                               FileOpenFlags flags) noexcept {
  const bool seek_end = strip_flag(&flags, FileOpenFlags::SeekEnd) &&
                        (flags & FileOpenFlags::Trunc) == FileOpenFlags::None;
  const int  fildes   = ::open(cstr_path, unix_flags(flags), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fildes == -1) [[unlikely]] {
    if (errno == ENOENT) {
      return File::OpenError::FileNotFound;
    }
    panic("::open() failed: {}", errno_str());
  }
  if (seek_end) {
    panic_if(lseek(fildes, 0, SEEK_END) == -1, "::lseek() failed: {}", errno_str());
  }
  return static_cast<unsigned int>(fildes);
}

static UnsignedResult<unsigned int, File::OpenError> open_file(const StringView& path,
                                                               FileOpenFlags     flags) noexcept {
  CString cstr(path);
  return open_file(cstr.data(), flags);
}

} // namespace internal

Error<File::OpenError> File::open(const StringView& path, FileOpenFlags flags) noexcept {
  const UnsignedResult<unsigned int, File::OpenError> result = internal::open_file(path, flags);
  if (result.is_error()) [[unlikely]] {
    handle_ = -1;
    return result.error();
  } else {
    handle_ = static_cast<int>(result.assert_ok());
    return NIL;
  }
}
Error<File::OpenError> File::open(const char* path, FileOpenFlags flags) noexcept {
  const UnsignedResult<unsigned int, File::OpenError> result = internal::open_file(path, flags);
  if (result.is_error()) [[unlikely]] {
    handle_ = -1;
    return result.error();
  } else {
    handle_ = static_cast<int>(result.assert_ok());
    return NIL;
  }
}

void File::close() noexcept {
  if (is_open()) {
    panic_if(::close(handle_) == -1, "::close() failed: {}", errno_str());
    handle_ = NIL_FILE_HANDLE;
  }
}

// NOLINTBEGIN(readability-make-member-function-const)
void File::write(const StringView& data) noexcept {
  panic_if(::write(handle_, data.data(), data.size()) == -1, "::write() failed: {}", errno_str());
}
void File::write(const BasicStringView<uchar>& data) noexcept {
  panic_if(::write(handle_, data.data(), data.size()) == -1, "::write() failed: {}", errno_str());
}
usize File::read(char* buf, usize size) noexcept {
  ssize_t result = ::read(handle_, buf, size);
  panic_if(result == -1, "::read() failed: {}", errno_str());
  return static_cast<usize>(result);
}
usize File::read(uchar* buf, usize size) noexcept {
  ssize_t result = ::read(handle_, buf, size);
  panic_if(result == -1, "::read() failed: {}", errno_str());
  return static_cast<usize>(result);
}
void File::seek_begin(isize offset) noexcept {
  off_t result = ::lseek(handle_, static_cast<off_t>(offset), SEEK_SET);
  panic_if(result == -1, "::lseek() failed: {}", errno_str());
  cursor_pos_ = static_cast<usize>(result);
}
void File::seek_end(isize offset) noexcept {
  off_t result = ::lseek(handle_, static_cast<off_t>(offset), SEEK_END);
  panic_if(result == -1, "::lseek() failed: {}", errno_str());
  cursor_pos_ = static_cast<usize>(result);
}
void File::seek_cursor(isize offset) noexcept {
  off_t result = ::lseek(handle_, static_cast<off_t>(offset), SEEK_CUR);
  panic_if(result == -1, "::lseek() failed: {}", errno_str());
  cursor_pos_ = static_cast<usize>(result);
}
// NOLINTEND(readability-make-member-function-const)

} // namespace base

#ifdef BASE_LSP_ONLY
template class base::BasicCString<char, 8>;
template base::BasicCString<char, 8>::BasicCString(const BasicStringView<char>& str);
#endif
