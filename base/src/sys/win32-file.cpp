#ifdef BASE_PLATFORM_WINDOWS

#include "base/containers/cstring.hpp"
#include "base/containers/error.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/sys/file.hpp"
#include "base/type_traits/limits.hpp"
#include "base/utilities/type_aliases.hpp"
#include "win32-utils.hpp"
#include <fileapi.h>
#include <intsafe.h>

namespace base {

[[nodiscard]] Error<File::OpenError> File::open(const char* path, FileOpenFlags flags) noexcept {
  close();
  Result<HANDLE, OpenError> result = open_file(path, flags);
  if (result.is_ok()) [[likely]] {
    handle_ = result.assert_ok();
    return NIL;
  }
  handle_ = NIL_FILE_HANDLE;
  return result.assert_err();
}

void File::close() noexcept {
  if (handle_ != NIL_FILE_HANDLE) {
    BASE_TRY_SYSCALL(CloseHandle(handle_));
  }
}

// NOLINTBEGIN(readability-make-member-function-const)
usize File::write(const BasicStringView<uchar>& data) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(WriteFile(handle_, data.data(), static_cast<DWORD>(data.size()), &ret, nullptr));
  return static_cast<usize>(ret);
}
usize File::read(uchar* buf, usize size) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(ReadFile(handle_, buf, static_cast<DWORD>(size), &ret, nullptr));
  return static_cast<usize>(ret);
}

void File::seek_begin(isize offset) noexcept {
  auto low  = static_cast<LONG>(offset);
  auto high = static_cast<LONG>(offset / Limits<LONG>::max);
  BASE_TRY_SYSCALL(SetFilePointer(handle_, low, high > 0 ? &high : nullptr, FILE_BEGIN));
}
void File::seek_end(isize offset) noexcept {
  auto low  = static_cast<LONG>(offset);
  auto high = static_cast<LONG>(offset / Limits<LONG>::max);
  BASE_TRY_SYSCALL(SetFilePointer(handle_, low, high > 0 ? &high : nullptr, FILE_END));
}
void File::seek_cursor(isize offset) noexcept {
  auto low  = static_cast<LONG>(offset);
  auto high = static_cast<LONG>(offset / Limits<LONG>::max);
  BASE_TRY_SYSCALL(SetFilePointer(handle_, low, high > 0 ? &high : nullptr, FILE_CURRENT));
}
// NOLINTEND(readability-make-member-function-const)

} // namespace base

#endif
