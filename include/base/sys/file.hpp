#pragma once

#include "base/containers/string_view.hpp"
#include "base/sys/common.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class File {
public:
  constexpr File() noexcept = default;
  explicit File(const StringView& path, FileOpenFlags flags) { open(path, flags); }
  explicit File(const char* path, FileOpenFlags flags) { open(path, flags); }
  File(const File&) = delete;
  File(File&& other) noexcept;

  ~File() noexcept { close(); }

  File& operator=(const File&) = delete;
  File& operator=(File&& other) noexcept;

  [[nodiscard]] bool is_open() const noexcept { return handle_ != NIL_FILE_HANDLE; }

  void open(const StringView& path, FileOpenFlags flags);
  void open(const char* path, FileOpenFlags flags);
  void close();

  void write(const StringView& data);
  void write(const BasicStringView<uchar>& data);
  void read(char* buf, usize size);
  void read(uchar* buf, usize size);

  [[nodiscard]] isize get_cursor();

  isize seek_begin(isize offset);
  isize seek_end(isize offset);
  isize seek_cursor(isize offset);

private:
  explicit constexpr File(FileHandle handle) noexcept : handle_(handle) {}

  FileHandle handle_ = NIL_FILE_HANDLE;

  friend class Process;
};

} // namespace base
