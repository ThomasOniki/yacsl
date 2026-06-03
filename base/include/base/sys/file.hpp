#pragma once

#include "base/containers/cstring.hpp"
#include "base/containers/error.hpp"
#include "base/containers/string_view.hpp"
#include "base/sys/common.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class File {
public:
  constexpr File() noexcept = default;
  explicit File(const StringView& path, FileOpenFlags flags) { open(path, flags).expect_nil(); }
  explicit File(const char* path, FileOpenFlags flags) { open(path, flags).expect_nil(); }
  File(const File&) = delete;
  File(File&& other) noexcept;

  ~File() noexcept { close(); }

  File& operator=(const File&) = delete;
  File& operator=(File&& other) noexcept;

  [[nodiscard]] bool is_open() const noexcept { return handle_ != NIL_FILE_HANDLE; }

  enum class OpenError : u8 {
    FileNotFound = 1,
  };

  [[nodiscard]] Error<OpenError> open(const char* path, FileOpenFlags flags) noexcept;

  [[nodiscard]] Error<OpenError> open(const StringView& path, FileOpenFlags flags) noexcept {
    CString cstr(path);
    return open(cstr.data(), flags);
  }

  void close() noexcept;

  usize write(const BasicStringView<uchar>& data) noexcept;

  usize write(const StringView& data) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return write(BasicStringView<uchar>(reinterpret_cast<const uchar*>(data.data()), data.size()));
  }
  usize read(uchar* buf, usize size) noexcept;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  usize read(char* buf, usize size) noexcept { return read(reinterpret_cast<uchar*>(buf), size); }

  [[nodiscard]] usize get_cursor() const noexcept { return cursor_pos_; }

  void seek_begin(isize offset) noexcept;
  void seek_end(isize offset) noexcept;
  void seek_cursor(isize offset) noexcept;

private:
  explicit constexpr File(FileHandle handle) noexcept : handle_(handle) {}

  FileHandle handle_     = NIL_FILE_HANDLE;
  usize      cursor_pos_ = 0;

  friend class Process;
};

} // namespace base
