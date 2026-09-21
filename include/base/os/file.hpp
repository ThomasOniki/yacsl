#pragma once

#include "base/containers/result.hpp"
#include "base/containers/string.hpp"
#include "base/ranges/slice.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_aliases.hpp"
#include "base/utils/isize.hpp"

namespace base::os {

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

  enum class OpenError : u8 {
    FileNotFound = 1,
  };
  [[nodiscard]] static containers::Result<File, OpenError>
  open(char const* path, Flags flags) noexcept;
  [[nodiscard]] static containers::Result<File, OpenError>
  open(ranges::StringView path, Flags flags) noexcept {
    return open(containers::String(path).c_str(), flags);
  }

  ~File() noexcept { close(); }

  File(File const&) = delete;
  File(File&& other) noexcept
      : handle_(other.handle_), cursor_pos_(other.cursor_pos_),
        is_tty_(other.is_tty_) {
    other.handle_ = NIL_HANDLE;
  }

  File& operator=(File const&) = delete;

  File& operator=(this File& self BASE_LIFETIMEBOUND, File&& other) noexcept {
    self.close();
    self.handle_     = other.handle_;
    self.cursor_pos_ = other.cursor_pos_;
    self.is_tty_     = other.is_tty_;
    other.handle_    = NIL_HANDLE;
    return self;
  }

  // seek/close don't do anything. This doesn't duplicate the standard streams,
  // it just sets handle_.
  [[nodiscard]] static File open_stdin() noexcept;
  [[nodiscard]] static File open_stdout() noexcept;
  [[nodiscard]] static File open_stderr() noexcept;

  usize write(this File& self, ranges::Slice<uchar const> data) noexcept;
  usize read(this File& self, ranges::Slice<uchar> buffer) noexcept;

  [[nodiscard]] usize get_cursor(this File const& self) noexcept {
    return self.cursor_pos_;
  }

  [[nodiscard]] bool is_tty(this File const& self) noexcept;
  void               invalidate_is_tty_cache(this File& self) noexcept {
    self.is_tty_ = IsTtyFlag::NotComputed;
  }

  void seek_begin(this File& self, utils::isize offset) noexcept;
  void seek_end(this File& self, utils::isize offset) noexcept;
  void seek_cursor(this File& self, utils::isize offset) noexcept;

#ifdef _WIN32
  using Handle                       = void*;
  static constexpr Handle NIL_HANDLE = nullptr;
#else
  using Handle                       = int;
  static constexpr Handle NIL_HANDLE = -1;
#endif

  [[nodiscard]] constexpr Handle handle(this File const& self) noexcept {
    return self.handle_;
  }

private:
  void close(this File& self) noexcept;

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
  return File::Flags(
      static_cast<u8>(static_cast<u8>(lhs) | static_cast<u8>(rhs)));
}
constexpr File::Flags operator&(File::Flags lhs, File::Flags rhs) noexcept {
  return File::Flags(
      static_cast<u8>(static_cast<u8>(lhs) & static_cast<u8>(rhs)));
}
constexpr File::Flags operator~(File::Flags flags) noexcept {
  return File::Flags(~static_cast<u8>(flags));
}

} // namespace base::os
