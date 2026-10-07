#include "yacsl/utils/nil.hpp"
#ifdef _WIN32

#include "win32-utils.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/meta/limits.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/isize.hpp"
#include <accctrl.h>
#include <aclapi.h>
#include <errhandlingapi.h>
#include <fileapi.h>
#include <intsafe.h>
#include <windows.h>
#include <winnt.h>

namespace yacsl::os {

containers::Result<File, File::OpenError>
File::open(char const* path, File::OpenFlags flags) noexcept {
  containers::Result<HANDLE, OpenError> handle = open_file_handle(path, flags);
  if (handle.is_error()) [[unlikely]] {
    return handle.error_unchecked();
  }
  File opened;
  opened.handle_ = handle.value_unchecked();

  if (strip_file_open_flag(&flags, File::OpenFlags::SeekEnd)) {
    LONG high = 0;
    opened.cursor_pos_ =
        static_cast<usize>(SetFilePointer(opened.handle_, 0, &high, FILE_END));
    opened.cursor_pos_ += static_cast<usize>(high) * meta::Limits<LONG>::MAX;
    if (opened.cursor_pos_ == static_cast<usize>(INVALID_SET_FILE_POINTER)) {
      DWORD const err = GetLastError();
      if (err != NO_ERROR) {
        debug::panic("SetFilePointer failed: {}", win32_err_str(err));
      }
    }
  }

  return opened;
}

void File::close(this File& self) noexcept {
  if (self.handle_ != NIL_HANDLE) {
    YACSL_TRY_SYSCALL(CloseHandle(self.handle_));
  }
}

File File::open_stdin() noexcept {
  File opened;
  opened.handle_ = GetStdHandle(STD_INPUT_HANDLE);
  return opened;
}
File File::open_stdout() noexcept {
  File opened;
  opened.handle_ = GetStdHandle(STD_OUTPUT_HANDLE);
  return opened;
}
File File::open_stderr() noexcept {
  File opened;
  opened.handle_ = GetStdHandle(STD_ERROR_HANDLE);
  return opened;
}

// NOLINTBEGIN(readability-make-member-function-const)
usize File::write(this File& self, ranges::Slice<u8 const> data) noexcept {
  DWORD ret = 0;
  YACSL_TRY_SYSCALL(WriteFile(self.handle_, data.data(),
                              static_cast<DWORD>(data.len()), &ret, nullptr));
  return static_cast<usize>(ret);
}
usize File::read(this File& self, ranges::Slice<u8> buffer) noexcept {
  DWORD ret = 0;
  YACSL_TRY_SYSCALL(ReadFile(self.handle_, buffer.data(),
                             static_cast<DWORD>(buffer.len()), &ret, nullptr));
  return static_cast<usize>(ret);
}

void File::seek_begin(this File& self, utils::isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / meta::Limits<LONG>::MAX);
  DWORD pos_low = SetFilePointer(self.handle_, low, &high, FILE_BEGIN);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      debug::panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  self.cursor_pos_ =
      static_cast<usize>(high) * meta::Limits<LONG>::MAX + pos_low;
}
void File::seek_end(this File& self, utils::isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / meta::Limits<LONG>::MAX);
  DWORD pos_low = SetFilePointer(self.handle_, low, &high, FILE_END);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      debug::panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  self.cursor_pos_ =
      static_cast<usize>(high) * meta::Limits<LONG>::MAX + pos_low;
}
void File::seek_cursor(this File& self, utils::isize offset) noexcept {
  auto  low     = static_cast<LONG>(offset);
  auto  high    = static_cast<LONG>(offset / meta::Limits<LONG>::MAX);
  DWORD pos_low = SetFilePointer(self.handle_, low, &high, FILE_CURRENT);
  if (pos_low == INVALID_SET_FILE_POINTER) {
    DWORD const err = GetLastError();
    if (err != NO_ERROR) {
      debug::panic("SetFilePointer failed: {}", win32_err_str(err));
    }
  }
  self.cursor_pos_ =
      static_cast<usize>(high) * meta::Limits<LONG>::MAX + pos_low;
}
// NOLINTEND(readability-make-member-function-const)

bool File::is_tty(this File const& self) noexcept {
  if (self.is_tty_ == IsTtyFlag::NotComputed) {
    self.is_tty_ =
        static_cast<IsTtyFlag>(GetFileType(self.handle_) == FILE_TYPE_CHAR);
  }
  return static_cast<bool>(self.is_tty_);
}

[[nodiscard]] containers::Error<File::OpenError>
File::access(char const* path, AccessFlags flags) {
  SECURITY_DESCRIPTOR desc       = {};
  DWORD               needed_len = 0;
  if (GetFileSecurityA(path, DACL_SECURITY_INFORMATION, &desc, sizeof(desc),
                       &needed_len) == FALSE) {
    DWORD const err = GetLastError();
    switch (err) {
    case ERROR_FILE_NOT_FOUND:
      return OpenError::FileNotFound;
    case ERROR_NOACCESS:
      return OpenError::PermissionDenied;
    default:
      debug::panic("GetFileSecurityA() failed: {}", win32_err_str(err));
    }
  }
  debug::assert_eq(sizeof(desc), needed_len);
  TRUSTEE_A trustee = {
      .pMultipleTrustee         = nullptr,
      .MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE,
      .TrusteeForm              = TRUSTEE_IS_NAME,
      .TrusteeType              = TRUSTEE_IS_USER,
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
      .ptstrName = const_cast<char*>("CURRENT_USER"),
  };
  ACCESS_MASK access = 0;
  YACSL_TRY_SYSCALL(GetEffectiveRightsFromAclA(desc.Dacl, &trustee, &access));

  if (static_cast<u8>(flags & AccessFlags::Read) > 0 &&
      (access & GENERIC_READ) == 0) {
    return OpenError::PermissionDenied;
  }
  if (static_cast<u8>(flags & AccessFlags::Write) > 0 &&
      (access & GENERIC_WRITE) == 0) {
    return OpenError::PermissionDenied;
  }
  if (static_cast<u8>(flags & AccessFlags::Execute) > 0 &&
      (access & GENERIC_EXECUTE) == 0) {
    return OpenError::PermissionDenied;
  }
  return utils::NIL;
}

} // namespace yacsl::os

#endif
