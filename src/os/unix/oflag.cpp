#ifndef _WIN32

#include "yacsl/os/unix/oflag.hpp"
#include "yacsl/os/file.hpp"

namespace yacsl::os::unix {

int to_oflag(File::OpenFlags flags) noexcept {
  unsigned int ret = 0;

  if (strip_file_open_flag(&flags, File::OpenFlags::Truncate)) {
    ret |= O_TRUNC;
  }
  if (strip_file_open_flag(&flags, File::OpenFlags::Create)) {
    ret |= O_CREAT;
  }
  if (strip_file_open_flag(&flags, File::OpenFlags::Exclusive)) {
    ret |= O_EXCL;
  }
  if (!strip_file_open_flag(&flags, File::OpenFlags::Inheritable)) {
    ret |= O_CLOEXEC;
#ifdef O_CLOFORK
    ret |= O_CLOFORK;
#endif
  }
  if (flags == File::OpenFlags::Read) {
    ret |= O_RDONLY;
  } else if (flags == File::OpenFlags::Write) {
    ret |= O_WRONLY;
  } else {
    ret |= O_RDWR;
  }
  return static_cast<int>(ret);
}

} // namespace yacsl::os::unix

#endif
