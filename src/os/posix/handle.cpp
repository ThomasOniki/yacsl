#ifndef _WIN32

#include "yacsl/os/posix/handle.hpp"
#include "yacsl/containers/result.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/os/file.hpp"
#include "yacsl/os/posix/errno.hpp"
#include "yacsl/os/posix/oflag.hpp"
#include <errno.h>

namespace yacsl::os::posix {

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

} // namespace yacsl::os::posix

#endif
