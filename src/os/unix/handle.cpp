#ifndef _WIN32

#include "yacsl/os/unix/handle.hpp"
#include "yacsl/os/unix/errno.hpp"
#include "yacsl/os/unix/oflag.hpp"

namespace yacsl::os::unix {

containers::Result<int, File::OpenError>
open_file_handle(char const* path, File::OpenFlags flags) {
  int handle = ::open(path, to_oflag(flags), 0644);
  if (handle == -1) [[unlikely]] {
    switch (errno) {
    case ENOENT:
      return File::OpenError::NotFound;
    case EEXIST:
      return File::OpenError::AlreadyExists;
    case EACCES:
      return File::OpenError::PermissionDenied;
    default:
      debug::panic("::open() failed: {}", errno_str());
    }
  }
  return handle;
}

} // namespace yacsl::os::unix

#endif
