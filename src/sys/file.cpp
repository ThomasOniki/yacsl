#include "base/sys/file.hpp"
#include "base/containers/cstring.hpp"
#include "base/containers/fwds.hpp"
#include "base/containers/string_view.hpp"
#include "base/sys/common.hpp"
#include "base/utilities/type_aliases.hpp"
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

namespace base {

namespace internal {

static bool strip_flag(FileOpenFlags* flags, FileOpenFlags flag) {
  const FileOpenFlags stripped = *flags & ~flag;
  if (stripped != *flags) {
    *flags = stripped;
    return true;
  }
  return false;
}

static int unix_flags(FileOpenFlags flags) {
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

static int open_file(const char* cstr_path, FileOpenFlags flags) {
  const bool seek_end = strip_flag(&flags, FileOpenFlags::SeekEnd) &&
                        (flags & FileOpenFlags::Trunc) == FileOpenFlags::None;
  const int  fildes   = ::open(cstr_path, unix_flags(flags), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if (fildes == -1) [[unlikely]] {
    perror("::open failed");
    return NIL_FILE_HANDLE;
  }
  if (seek_end) {
    if (lseek(fildes, 0, SEEK_END) == -1) [[unlikely]] {
      perror("::lseek failed");
      return NIL_FILE_HANDLE;
    }
  }
  return fildes;
}

static int open_file(const StringView& path, FileOpenFlags flags) {
  CString cstr(path);
  return open_file(cstr.data(), flags);
}

} // namespace internal

void File::open(const StringView& path, FileOpenFlags flags) {
  handle_ = internal::open_file(path, flags);
}
void File::open(const char* path, FileOpenFlags flags) {
  handle_ = internal::open_file(path, flags);
}

void File::close() {
  if (is_open()) {
    if (::close(handle_) == -1) [[unlikely]] {
      perror("::close failed");
    }
    handle_ = NIL_FILE_HANDLE;
  }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void File::write(const StringView& data) {
  if (::write(handle_, data.data(), data.size()) == -1) [[unlikely]] {
    perror("::write() failed");
  }
}
// NOLINTNEXTLINE(readability-make-member-function-const)
void File::write(const BasicStringView<uchar>& data) {
  if (::write(handle_, data.data(), data.size()) == -1) [[unlikely]] {
    perror("::write() failed");
  }
}
// NOLINTNEXTLINE(readability-make-member-function-const)
void File::read(char* buf, usize size) {
  if (::read(handle_, buf, size) == -1) [[unlikely]] {
    perror("::read() failed");
  }
}
// NOLINTNEXTLINE(readability-make-member-function-const)
void File::read(uchar* buf, usize size) {
  if (::read(handle_, buf, size) == -1) [[unlikely]] {
    perror("::read() failed");
  }
}
// NOLINTNEXTLINE(readability-make-member-function-const)
isize File::get_cursor() {
  const auto result = static_cast<isize>(::lseek(handle_, 0, SEEK_CUR));
  if (result == -1) [[unlikely]] {
    perror("::lseek failed");
  }
  return result;
}
// NOLINTNEXTLINE(readability-make-member-function-const)
isize File::seek_begin(isize offset) {
  const auto result = static_cast<isize>(::lseek(handle_, offset, SEEK_SET));
  if (result == -1) [[unlikely]] {
    perror("::lseek() failed");
  }
  return result;
}
// NOLINTNEXTLINE(readability-make-member-function-const)
isize File::seek_end(isize offset) {
  const auto result = static_cast<isize>(::lseek(handle_, offset, SEEK_END));
  if (result == -1) [[unlikely]] {
    perror("::lseek() failed");
  }
  return result;
}
// NOLINTNEXTLINE(readability-make-member-function-const)
isize File::seek_cursor(isize offset) {
  const auto result = static_cast<isize>(::lseek(handle_, offset, SEEK_CUR));
  if (result == -1) [[unlikely]] {
    perror("::lseek() failed");
  }
  return result;
}

} // namespace base

#ifdef BASE_LSP_ONLY
template class base::BasicCString<char, 8>;
template base::BasicCString<char, 8>::BasicCString(const BasicStringView<char>& str);
#endif
