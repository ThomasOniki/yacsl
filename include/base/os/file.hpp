#pragma once

#include "base/containers/error.hpp"
#include "base/containers/string.hpp"
#include "base/ranges/slice.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/isize.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class File {
public:
  enum class Flags : u8 {
    None            = 0,
    Read            = 1 << 0,
    Write           = 1 << 1,
    Trunc           = 1 << 2,
    Create          = 1 << 3,
    SeekEnd         = 1 << 4,
    DontCloseOnExec = 1 << 5,
  };
  constexpr File() noexcept = default;
  explicit File(StringView path, Flags flags) { open(path, flags).expect_nil(); }
  explicit File(char const* path, Flags flags) { open(path, flags).expect_nil(); }
  File(File const&) = delete;
  File(File&& other) noexcept : handle_(other.handle_), cursor_pos_(other.cursor_pos_) {
    other.handle_ = NIL_HANDLE;
  }

  ~File() noexcept { close(); }

  File& operator=(File const&) = delete;
  File& operator=(File&& other) noexcept;

  [[nodiscard]] bool is_open() const noexcept { return handle_ != NIL_HANDLE; }

  enum class OpenError : u8 {
    FileNotFound = 1,
  };

  [[nodiscard]] Error<OpenError> open(char const* path, Flags flags) noexcept;

  [[nodiscard]] Error<OpenError> open(StringView path, Flags flags) noexcept {
    return open(String(path).c_str(), flags);
  }

  void close() noexcept;

  // seek/close don't do anything. This doesn't duplicate the standard streams, it just sets
  // handle_.
  void open_stdin() noexcept;
  void open_stdout() noexcept;
  void open_stderr() noexcept;

  usize write(Slice<uchar const> data) noexcept;
  usize read(Slice<uchar> buffer) noexcept;

  [[nodiscard]] usize get_cursor() const noexcept { return cursor_pos_; }

  [[nodiscard]] bool is_tty() const noexcept;
  void               invalidate_is_tty_cache() noexcept { is_tty_ = IsTtyFlag::NotComputed; }

  void seek_begin(isize offset) noexcept;
  void seek_end(isize offset) noexcept;
  void seek_cursor(isize offset) noexcept;

#ifdef _WIN32
  using Handle                       = void*;
  static constexpr Handle NIL_HANDLE = nullptr;
#else
  using Handle                       = int;
  static constexpr Handle NIL_HANDLE = -1;
#endif

  [[nodiscard]] constexpr Handle handle() const noexcept { return handle_; }

private:
  Handle handle_     = NIL_HANDLE;
  usize  cursor_pos_ = 0;

  enum class IsTtyFlag : u8 {
    False       = 0,
    True        = 1,
    NotComputed = 255,
  };
  mutable IsTtyFlag is_tty_ = IsTtyFlag::NotComputed;
};

constexpr File::Flags operator|(File::Flags lhs, File::Flags rhs) noexcept {
  return File::Flags(static_cast<u8>(static_cast<u8>(lhs) | static_cast<u8>(rhs)));
}
constexpr File::Flags operator&(File::Flags lhs, File::Flags rhs) noexcept {
  return File::Flags(static_cast<u8>(static_cast<u8>(lhs) & static_cast<u8>(rhs)));
}
constexpr File::Flags operator~(File::Flags flags) noexcept {
  return File::Flags(~static_cast<u8>(flags));
}

} // namespace base
