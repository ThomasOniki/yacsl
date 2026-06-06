#pragma once

#include "base/containers/cstring.hpp"
#include "base/containers/error.hpp"
#include "base/containers/initializer_list.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/containers/vector.hpp"
#include "base/logging/panic.hpp"
#include "base/sys/common.hpp"
#include "base/sys/file.hpp"
#include "base/type_traits/range_like.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class Process {
public:
  // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
  class Stdio {
  public:
    enum class Type : u8 {
      Buffer       = 0,
      ParentStdin  = 1,
      ParentStdout = 2,
      ParentStderr = 3,

      File = 254,
      Pipe = 255,
    };
    using enum Type;

    Stdio(const char* path, FileOpenFlags flags) noexcept
        : file_info_{.path = path, .flags = flags}, type_(File) {}

    Stdio(StringView path, FileOpenFlags flags) noexcept
        : file_info_{.path = nullptr, .flags = flags}, type_(File) {
      CString cstr(path);
      file_info_.path = cstr.data();
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    Stdio(Type type) noexcept : type_(type) {
      debug_panic_if(type == Pipe || type == File,
                     "Process::Stdio instances of types File and Pipe must be constructed with "
                     "Stdio(path, flags) and Stdio(pipe) respectively.");
    }

    Stdio(BASE_LIFETIMEBOUND Stdio& pipe) noexcept : pipe_(&pipe), type_(Pipe) {
      debug_panic_if(pipe.type_ != Buffer, "In Process::Stdio(Stdio& pipe), pipe must be a buffer");
      debug_panic_if(pipe.buffer_info_.process == nullptr,
                     "In Process::Stdio(Stdio& pipe) pipe must be a Stdio& acquired with "
                     "process.get_{stdout, stderr}.");
    }

    ~Stdio() noexcept = default;

    Stdio& operator=(const Stdio&) = delete;

    usize write(BasicStringView<uchar> data) noexcept;

    usize write(StringView data) noexcept {
      return write( // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          BasicStringView<uchar>(reinterpret_cast<const uchar*>(data.data()), data.size()));
    }

    usize read(uchar* buf, usize size) noexcept;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    usize read(char* buf, usize size) noexcept { return read(reinterpret_cast<uchar*>(buf), size); }

    void seek_begin(isize offset) noexcept;
    void seek_end(isize offset) noexcept;
    void seek_cursor(isize offset) noexcept;

    [[nodiscard]] usize get_cursor() const noexcept {
      assert_buffer("get_cursor");
      return buffer_info_.cursor_pos;
    }

  private:
    void assert_buffer(StringView method) const noexcept {
      debug_panic_if(type_ != Buffer,
                     "Process::Stdio::{} must be called on an instance that is a buffer.", method);
      debug_panic_if(buffer_info_.process == nullptr,
                     "Process::Stdio::{} must be called on an Stdio& acquired from process.get_{}.",
                     method, "{stdout, stderr}");
    }

  private:
    Stdio& operator=(Stdio&&) = default;
    Stdio(Stdio&&) noexcept   = default;
    Stdio() noexcept          = default;

  private:
    struct FileInfo {
      alignas(16) const char* path;
      FileOpenFlags flags;
    };
    struct BufferInfo {
      alignas(32) Process* process = nullptr;
      FileHandle handle;
      usize      cursor_pos;
    };

    union {
      FileInfo   file_info_;
      BufferInfo buffer_info_{};
      Stdio*     pipe_;
    };

    Type type_{0};

    friend class Process;
  };
  // NOLINTEND(cppcoreguidelines-pro-type-union-access)

  Process() noexcept = default;
  ~Process() noexcept { wait(); }

  Process(const Process&)            = delete;
  Process(Process&&)                 = delete;
  Process& operator=(const Process&) = delete;
  Process& operator=(Process&&)      = delete;

  enum class SpawnError : u8 {
    FileNotFound = 1,
  };
  Error<SpawnError> start(const char* const* argv, Stdio&& stdin_opts, Stdio&& stdout_opts,
                          Stdio&& stderr_opts) noexcept;

  Error<SpawnError> start(InitializerList<const char*> argv, Stdio&& stdin_opts,
                          Stdio&& stdout_opts, Stdio&& stderr_opts) noexcept {
    Vector<const char*, 6> cstrs;
    cstrs.reserve(argv.size());
    for (const char* str : argv) {
      cstrs.push_back(str);
    }
    cstrs.push_back(nullptr);
    return start(cstrs.data(), base::move(stdin_opts), base::move(stdout_opts),
                 base::move(stderr_opts));
  }

  Error<SpawnError> start(InitializerList<StringView> argv, Stdio&& stdin_opts, Stdio&& stdout_opts,
                          Stdio&& stderr_opts) noexcept {
    return start<InitializerList<StringView>>(argv, base::move(stdin_opts), base::move(stdout_opts),
                                              base::move(stderr_opts));
  }

  template <RangeOf<StringView> Range>
  Error<SpawnError> start(const Range& argv, Stdio&& stdin_opts, Stdio&& stdout_opts,
                          Stdio&& stderr_opts) noexcept {
    const Vector<CString, 6> cstrs(argv);
    Vector<const char*, 6>   cstr_ptrs;
    cstr_ptrs.reserve(cstrs.size() + 1);
    for (const CString& cstr : cstrs) {
      cstr_ptrs.push_back(cstr.data());
    }
    cstr_ptrs.push_back(nullptr);
    return start(cstr_ptrs.data(), base::move(stdin_opts), base::move(stdout_opts),
                 base::move(stderr_opts));
  }

  struct ExitStatus {
    alignas(8) u32 exit_code;
    Signal signal;
    Signal stopped;
  };

  ExitStatus wait() noexcept;

  void send_signal(Signal signal) noexcept;

  Stdio& get_stdin() noexcept BASE_LIFETIMEBOUND { return stdin_; }
  Stdio& get_stdout() noexcept BASE_LIFETIMEBOUND { return stdout_; }
  Stdio& get_stderr() noexcept BASE_LIFETIMEBOUND { return stderr_; }

private:
  void handle_piped_stdio(void* voidp) noexcept;
  void handle_file_stdio(void* voidp) noexcept;

private:
  struct OriginalStdio {
    alignas(16) FileHandle input;
    FileHandle output;
    FileHandle error;
  };

  ProcessHandle             handle_      = NIL_PROCESS_HANDLE;
  StackArray<FileHandle, 3> original_io_ = {NIL_FILE_HANDLE, NIL_FILE_HANDLE, NIL_FILE_HANDLE};
  StackArray<FileHandle, 2> stdin_pipe_  = {NIL_FILE_HANDLE, NIL_FILE_HANDLE};
  StackArray<FileHandle, 2> stdout_pipe_ = {NIL_FILE_HANDLE, NIL_FILE_HANDLE};
  StackArray<FileHandle, 2> stderr_pipe_ = {NIL_FILE_HANDLE, NIL_FILE_HANDLE};
  Stdio                     stdin_;
  Stdio                     stdout_;
  Stdio                     stderr_;

#ifdef _WIN32
  u32 group_;
#endif
};

} // namespace base
