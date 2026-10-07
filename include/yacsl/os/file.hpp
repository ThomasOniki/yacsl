#pragma once

#include "yacsl/containers/result.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/compiler/export.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/isize.hpp"

namespace yacsl::os {

class File {
public:
  enum class OpenFlags : u8 {
    None            = 0,
    Read            = 1U << 0U,
    Write           = 1U << 1U,
    Trunc           = 1U << 2U,
    Create          = 1U << 3U,
    SeekEnd         = 1U << 4U,
    DontCloseOnExec = 1U << 5U,
  };
  constexpr File() noexcept = default;

  enum class OpenError : u8 {
    None,
    FileNotFound,
    PermissionDenied,
  };

  [[nodiscard]] static containers::Result<File, OpenError>
  open(char const* path, OpenFlags flags) noexcept YACSL_API;
  [[nodiscard]] static containers::Result<File, OpenError>
  open(ranges::StringView path, OpenFlags flags) noexcept {
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

  File& operator=(this File& self YACSL_LIFETIMEBOUND, File&& other) noexcept {
    self.close();
    self.handle_     = other.handle_;
    self.cursor_pos_ = other.cursor_pos_;
    self.is_tty_     = other.is_tty_;
    other.handle_    = NIL_HANDLE;
    return self;
  }

  // This doesn't duplicate the standard streams, it just sets handle_ to the
  // sentinel values for the standard handles.
  [[nodiscard]] static File open_stdin() noexcept YACSL_API;
  [[nodiscard]] static File open_stdout() noexcept YACSL_API;
  [[nodiscard]] static File open_stderr() noexcept YACSL_API;

  usize write(this File& self, ranges::Slice<u8 const> data) noexcept YACSL_API;
  usize read(this File& self, ranges::Slice<u8> buffer) noexcept YACSL_API;

  [[nodiscard]] usize get_cursor(this File const& self) noexcept {
    return self.cursor_pos_;
  }

  [[nodiscard]] bool is_tty(this File const& self) noexcept YACSL_API;
  void               invalidate_is_tty_cache(this File& self) noexcept {
    self.is_tty_ = IsTtyFlag::NotComputed;
  }

  void seek_begin(this File& self, utils::isize offset) noexcept YACSL_API;
  void seek_end(this File& self, utils::isize offset) noexcept YACSL_API;
  void seek_cursor(this File& self, utils::isize offset) noexcept YACSL_API;

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
  void close(this File& self) noexcept YACSL_API;

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

constexpr File::OpenFlags operator|(File::OpenFlags lhs,
                                    File::OpenFlags rhs) noexcept {
  return File::OpenFlags(
      static_cast<u8>(static_cast<u8>(lhs) | static_cast<u8>(rhs)));
}
constexpr File::OpenFlags operator&(File::OpenFlags lhs,
                                    File::OpenFlags rhs) noexcept {
  return File::OpenFlags(
      static_cast<u8>(static_cast<u8>(lhs) & static_cast<u8>(rhs)));
}
constexpr File::OpenFlags operator~(File::OpenFlags flags) noexcept {
  return File::OpenFlags(~static_cast<u8>(flags));
}

} // namespace yacsl::os
