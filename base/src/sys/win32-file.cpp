#ifdef BASE_PLATFORM_WINDOWS

#include "base/containers/cstring.hpp"
#include "base/containers/error.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/sys/file.hpp"
#include "base/type_traits/limits.hpp"
#include "win32-utils.hpp"
#include <fileapi.h>
#include <intsafe.h>

namespace base {

static bool strip_flag(FileOpenFlags* flags, FileOpenFlags flag) noexcept {
  const FileOpenFlags stripped = *flags & ~flag;
  if (stripped != *flags) {
    *flags = stripped;
    return true;
  }
  return false;
}

Error<File::OpenError> File::open(const char* path, FileOpenFlags flags) noexcept {
  DWORD desired_access = 0;
  if (strip_flag(&flags, FileOpenFlags::Read)) {
    desired_access |= GENERIC_READ;
  }
  if (strip_flag(&flags, FileOpenFlags::Write)) {
    desired_access |= GENERIC_WRITE;
  }
  SECURITY_ATTRIBUTES security_attr{
      .nLength              = sizeof(SECURITY_ATTRIBUTES),
      .lpSecurityDescriptor = nullptr,
      .bInheritHandle       = strip_flag(&flags, FileOpenFlags::DontCloseOnExec),
  };
  const bool truncate = strip_flag(&flags, FileOpenFlags::Trunc);
  const bool create   = strip_flag(&flags, FileOpenFlags::Create);

  DWORD creation_disposition = 0;

  if (create) {
    if (truncate) {
      creation_disposition = CREATE_ALWAYS;
    } else {
      creation_disposition = OPEN_ALWAYS;
    }
  } else {
    if (truncate) {
      creation_disposition = TRUNCATE_EXISTING;
    } else {
      creation_disposition = OPEN_EXISTING;
    }
  }

  handle_ =
      CreateFileA(path, desired_access, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                  &security_attr, creation_disposition, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (handle_ != INVALID_HANDLE_VALUE) {
    if (strip_flag(&flags, FileOpenFlags::SeekEnd)) {
      seek_end(0);
    }
    return NIL;
  }

  handle_ = nullptr;

  const DWORD err = GetLastError();
  switch (err) {
  case ERROR_FILE_NOT_FOUND:
    return OpenError::FileNotFound;
  default:
    panic("CreateFileA() failed: {}", win32_err_str(err));
  }
}

void File::close() noexcept {
  panic_if(CloseHandle(handle_) == FALSE, "CloseHandle() failed: {})", win32_err_str());
}

// NOLINTBEGIN(readability-make-member-function-const)
usize File::write(const BasicStringView<uchar>& data) noexcept {
  DWORD ret = 0;
  panic_if(WriteFile(handle_, data.data(), static_cast<DWORD>(data.size()), &ret, nullptr) == FALSE,
           "WriteFile() failed: {}", win32_err_str());
  return static_cast<usize>(ret);
}
usize File::read(uchar* buf, usize size) noexcept {
  DWORD ret = 0;
  panic_if(ReadFile(handle_, buf, static_cast<DWORD>(size), &ret, nullptr) == FALSE,
           "ReadFile() failed: {}", win32_err_str());
  return static_cast<usize>(ret);
}

void File::seek_begin(isize offset) noexcept {
  auto low  = static_cast<LONG>(offset);
  auto high = static_cast<LONG>(offset / Limits<LONG>::max);
  SetFilePointer(handle_, low, high > 0 ? &high : nullptr, FILE_BEGIN);
}
void File::seek_end(isize offset) noexcept {
  auto low  = static_cast<LONG>(offset);
  auto high = static_cast<LONG>(offset / Limits<LONG>::max);
  SetFilePointer(handle_, low, high > 0 ? &high : nullptr, FILE_END);
}
void File::seek_cursor(isize offset) noexcept {
  auto low  = static_cast<LONG>(offset);
  auto high = static_cast<LONG>(offset / Limits<LONG>::max);
  SetFilePointer(handle_, low, high > 0 ? &high : nullptr, FILE_CURRENT);
}
// NOLINTEND(readability-make-member-function-const)

} // namespace base

#endif
