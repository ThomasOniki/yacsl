#pragma once

#ifndef __EMSCRIPTEN__
#include "base/attributes.hpp"
#include "base/containers/error.hpp"
#include "base/containers/string.hpp"
#include "base/containers/vector.hpp"
#include "base/os/common.hpp"
#include "base/os/file.hpp"
#include "base/ranges/initializer_list.hpp"
#include "base/ranges/slice.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/range_like.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class Process {
public:
  class Input;
  class Output;

  enum class StdioType : u8 {
    Null,
    Inherit,
    Pipe,
  };

private:
  template <bool IsInput> class StdioInit {
  public:
    constexpr StdioInit() noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr StdioInit(StdioType type) noexcept : type_(type) {}

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr StdioInit(Input& pipe) noexcept
      requires(!IsInput)
        : type_(TYPE_OTHER_PIPE), other_pipe_handle_(pipe.handle_) {
      pipe.handle_ = NIL_FILE_HANDLE;
    }
    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr StdioInit(Output& pipe) noexcept
      requires(IsInput)
        : type_(TYPE_OTHER_PIPE), other_pipe_handle_(pipe.handle_) {
      pipe.handle_ = NIL_FILE_HANDLE;
    }

    constexpr StdioInit(const char* path, File::Flags flags) noexcept
        : type_(TYPE_FILE), file_info_{.path = path, .flags = flags} {}
    constexpr StdioInit(StringView path, File::Flags flags) noexcept
        : type_(TYPE_FILE),
          file_info_{.path  = (BasicString<char, 0, MainArenaAllocator>(path).c_str()),
                     .flags = flags} {}

  private:
    friend class Process;

    StdioType type_ = StdioType::Null;
    struct FileInfo {
      alignas(16) const char* path = nullptr;
      File::Flags flags        = File::Flags::None;
    };
    union {
      FileInfo   file_info_;
      FileHandle other_pipe_handle_;
      Nil        dummy_ = NIL;
    };

    static constexpr auto TYPE_FILE       = StdioType{254};
    static constexpr auto TYPE_OTHER_PIPE = StdioType{255};
  };

public:
  class Input {
  public:
    void write(Slice<const uchar> bytes) noexcept;

    Input(const Input&)            = delete;
    Input(Input&&)                 = delete;
    Input& operator=(const Input&) = delete;
    Input& operator=(Input&&)      = delete;
    ~Input() noexcept              = default;

    using Init = StdioInit<true>;
    using enum StdioType;

  private:
    friend class Process;

    Input() noexcept = default;

  private:
    FileHandle handle_ = NIL_FILE_HANDLE;
  };

  class Output {
  public:
    Output(const Output&)            = delete;
    Output(Output&&)                 = delete;
    Output& operator=(const Output&) = delete;
    Output& operator=(Output&&)      = delete;
    ~Output() noexcept               = default;

    // returns 0 on EOF, between 1 and buffer.size() when sucessful.
    usize read(Slice<uchar> buffer) noexcept;

    using Init = StdioInit<false>;
    using enum StdioType;

  private:
    friend class Process;

    Output() noexcept = default;

  private:
    FileHandle handle_ = NIL_FILE_HANDLE;
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

  struct Stdio {
    alignas(128) Input::Init input;
    Output::Init output;
    Output::Init error;
  };

  [[nodiscard]] Error<SpawnError> start(const char* const* argv, const Stdio& stdio) noexcept;

  [[nodiscard]] Error<SpawnError> start(InitializerList<const char*> argv,
                                        const Stdio&                 stdio) noexcept {
    Vector<const char*, 6> cstrs;
    cstrs.reserve(argv.size());
    for (const char* str : argv) {
      cstrs.push_back(str);
    }
    cstrs.push_back(nullptr);
    return start(cstrs.data(), stdio);
  }

  [[nodiscard]] Error<SpawnError> start(InitializerList<StringView> argv,
                                        const Stdio&                stdio) noexcept {
    return start<InitializerList<StringView>>(argv, stdio);
  }

  template <RangeOf<StringView> Range>
  [[nodiscard]] Error<SpawnError> start(const Range& argv, const Stdio& stdio) noexcept {
    Vector<BasicString<char, 16>, 6> cstrs(argv);
    Vector<const char*, 6>           cstr_ptrs;
    cstr_ptrs.reserve(cstrs.size() + 1);
    for (auto& cstr : cstrs) {
      cstr_ptrs.push_back(cstr.c_str());
    }
    cstr_ptrs.push_back(nullptr);
    return start(cstr_ptrs.data(), stdio);
  }

  struct ExitStatus {
    alignas(64) String unread_stdout;
    String unread_stderr;
    u32    exit_code;
    Signal signal;
    Signal stopped;
  };

  ExitStatus wait() noexcept;

  void send_signal(Signal signal) noexcept;

  Input&  get_stdin() noexcept BASE_LIFETIMEBOUND { return stdin_input_; }
  Output& get_stdout() noexcept BASE_LIFETIMEBOUND { return stdout_output_; }
  Output& get_stderr() noexcept BASE_LIFETIMEBOUND { return stderr_output_; }

private:
  void create_pipes(void* pvoid_state) noexcept;
  void open_files(void* pvoid_state) noexcept;
  void cleanup_after_spawn(void* pvoid_state) noexcept;
  void cleanup_before_wait(ExitStatus* status) noexcept;

private:
  ProcessHandle handle_      = NIL_PROCESS_HANDLE;
  StdioType     stdin_type_  = Input::Inherit;
  StdioType     stdout_type_ = Output::Inherit;
  StdioType     stderr_type_ = Output::Inherit;

  Input  stdin_input_;
  Output stdout_output_;
  Output stderr_output_;

#ifdef _WIN32
  u32 group_;
#endif
};

} // namespace base

#endif
