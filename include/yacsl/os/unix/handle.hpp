#pragma once

#include "yacsl/containers/result.hpp"
#include "yacsl/os/file.hpp"

namespace yacsl::os::unix {

// SeekEnd will be ignored
containers::Result<int, File::OpenError>
open_file_handle(char const* path, File::OpenFlags flags);

} // namespace yacsl::os::unix
