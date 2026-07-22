#ifdef _WIN32
#include <fileapi.h>
#include <intsafe.h>
#include <windows.h>
#endif

#ifdef _WIN32

namespace base {

[[nodiscard]] Error<File::OpenError> File::open(const char* path, FileOpenFlags flags) noexcept {
  close();
  const Result<HANDLE, OpenError> result = open_file(path, flags);
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

void File::open_stdin() noexcept { handle_ = GetStdHandle(STD_INPUT_HANDLE); }
void File::open_stdout() noexcept { handle_ = GetStdHandle(STD_OUTPUT_HANDLE); }
void File::open_stderr() noexcept { handle_ = GetStdHandle(STD_ERROR_HANDLE); }

// NOLINTBEGIN(readability-make-member-function-const)
usize File::write(Slice<const uchar> data) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(WriteFile(handle_, data.data(), static_cast<DWORD>(data.size()), &ret, nullptr));
  return static_cast<usize>(ret);
}
usize File::read(Slice<uchar> buffer) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(
      ReadFile(handle_, buffer.data(), static_cast<DWORD>(buffer.size()), &ret, nullptr));
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
