#pragma once

#include "base/containers/error.hpp"
#include "base/containers/string.hpp"
#include "base/os/common.hpp"
#include "base/os/process.hpp"
#include "base/ranges/slice.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/isize.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class File {
public:
  constexpr File() noexcept = default;
  explicit File(StringView path, FileOpenFlags flags) { open(path, flags).expect_nil(); }
  explicit File(const char* path, FileOpenFlags flags) { open(path, flags).expect_nil(); }
  File(const File&) = delete;
  File(File&& other) noexcept : handle_(other.handle_), cursor_pos_(other.cursor_pos_) {
    other.handle_ = NIL_FILE_HANDLE;
  }

  ~File() noexcept { close(); }

  File& operator=(const File&) = delete;
  File& operator=(File&& other) noexcept;

  [[nodiscard]] bool is_open() const noexcept { return handle_ != NIL_FILE_HANDLE; }

  enum class OpenError : u8 {
    FileNotFound = 1,
  };

  [[nodiscard]] Error<OpenError> open(const char* path, FileOpenFlags flags) noexcept;

  [[nodiscard]] Error<OpenError> open(StringView path, FileOpenFlags flags) noexcept {
    return open(String(path).c_str(), flags);
  }

  void close() noexcept;

  // seek/close don't do anything. This doesn't duplicate the standard streams, it just sets
  // handle_.
  void open_stdin() noexcept;
  void open_stdout() noexcept;
  void open_stderr() noexcept;

  usize write(Slice<const uchar> data) noexcept;
  usize read(Slice<uchar> buffer) noexcept;

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
