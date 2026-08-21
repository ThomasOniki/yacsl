#pragma once

#ifndef __EMSCRIPTEN__
#include "base/attributes.hpp"
#include "base/containers/error.hpp"
#include "base/containers/string.hpp"
#include "base/containers/vector.hpp"
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

    constexpr StdioInit(StdioType type) noexcept : type_(type) {}

    constexpr StdioInit(Input& pipe) noexcept
      requires(!IsInput)
        : type_(TYPE_OTHER_PIPE), other_pipe_handle_(pipe.handle_) {
      pipe.handle_ = File::NIL_HANDLE;
    }

    constexpr StdioInit(Output& pipe) noexcept
      requires IsInput
        : type_(TYPE_OTHER_PIPE), other_pipe_handle_(pipe.handle_) {
      pipe.handle_ = File::NIL_HANDLE;
    }

    constexpr StdioInit(char const* path, File::Flags flags) noexcept
        : type_(TYPE_FILE), file_info_{.path = path, .flags = flags} {}
    constexpr StdioInit(StringView path, File::Flags flags) noexcept
        : type_(TYPE_FILE),
          file_info_{.path  = (BasicString<char, 0, MainArenaAllocator>(path).c_str()),
                     .flags = flags} {}

  private:
    friend class Process;

    StdioType type_ = StdioType::Null;
    struct FileInfo {
      alignas(16) char const* path = nullptr;
      File::Flags flags            = File::Flags::None;
    };
    union {
      FileInfo     file_info_;
      File::Handle other_pipe_handle_;
      Nil          dummy_ = NIL;
    };

    static constexpr auto TYPE_FILE       = StdioType{254};
    static constexpr auto TYPE_OTHER_PIPE = StdioType{255};
  };

public:
  class Input {
  public:
    void write(Slice<uchar const> bytes) noexcept;

    Input(Input const&)            = delete;
    Input(Input&&)                 = delete;
    Input& operator=(Input const&) = delete;
    Input& operator=(Input&&)      = delete;
    ~Input() noexcept              = default;

    using Init = StdioInit<true>;
    using enum StdioType;

  private:
    friend class Process;

    Input() noexcept = default;

  private:
    File::Handle handle_ = File::NIL_HANDLE;
  };

  class Output {
  public:
    Output(Output const&)            = delete;
    Output(Output&&)                 = delete;
    Output& operator=(Output const&) = delete;
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
    File::Handle handle_ = File::NIL_HANDLE;
  };

  Process() noexcept = default;
  ~Process() noexcept { wait(); }

  Process(Process const&)            = delete;
  Process(Process&&)                 = delete;
  Process& operator=(Process const&) = delete;
  Process& operator=(Process&&)      = delete;

  enum class SpawnError : u8 {
    FileNotFound = 1,
  };

  struct Stdio {
    alignas(128) Input::Init input;
    Output::Init output;
    Output::Init error;
  };

  [[nodiscard]] Error<SpawnError> start(char const* const* argv, Stdio const& stdio) noexcept;

  [[nodiscard]] Error<SpawnError> start(InitializerList<char const*> argv,
                                        Stdio const&                 stdio) noexcept {
    Vector<char const*, 6> cstrs;
    cstrs.reserve(argv.size());
    for (char const* str : argv) {
      cstrs.push_back(str);
    }
    cstrs.push_back(nullptr);
    return start(cstrs.data(), stdio);
  }

  [[nodiscard]] Error<SpawnError> start(InitializerList<StringView> argv,
                                        Stdio const&                stdio) noexcept {
    return start<InitializerList<StringView>>(argv, stdio);
  }

  template <RangeOf<StringView> Range>
  [[nodiscard]] Error<SpawnError> start(Range const& argv, Stdio const& stdio) noexcept {
    Vector<BasicString<char, 16>, 6> cstrs(argv);
    Vector<char const*, 6>           cstr_ptrs;
    cstr_ptrs.reserve(cstrs.size() + 1);
    for (auto& cstr : cstrs) {
      cstr_ptrs.push_back(cstr.c_str());
    }
    cstr_ptrs.push_back(nullptr);
    return start(cstr_ptrs.data(), stdio);
  }

  enum class Signal : u8 {
    SigHup    = 1,  // terminate process    terminal line hangup
    SigInt    = 2,  // terminate process    interrupt program
    SigQuit   = 3,  // create core image    quit program
    SigIll    = 4,  // create core image    illegal instruction
    SigTrap   = 5,  // create core image    trace trap
    SigAbrt   = 6,  // create core image    abort program (formerly SIGIOT)
    SigEmt    = 7,  // create core image    emulate instruction executed
    SigFpe    = 8,  // create core image    floating-point exception
    SigKill   = 9,  // terminate process    kill program
    SigBus    = 10, // create core image    bus error
    SigSegv   = 11, // create core image    segmentation violation
    SigSys    = 12, // create core image    non-existent system call invoked
    SigPipe   = 13, // terminate process    write on a pipe with no reader
    SigAlrm   = 14, // terminate process    real-time timer expired
    SigTerm   = 15, // terminate process    software termination signal
    SigUrg    = 16, // discard signal       urgent condition present on socket
    SigStop   = 17, // stop process         stop (cannot be caught or ignored)
    SigTstp   = 18, // stop process         stop signal generated from keyboard
    SigCont   = 19, // discard signal       continue after stop
    SigChld   = 20, // discard signal       child status has changed
    SigTtin   = 21, // stop process         background read attempted from control terminal
    SigTtou   = 22, // stop process         background write attempted to control terminal
    SigIo     = 23, // discard signal       I/O is possible on a descriptor (see fcntl(2))
    SigXcpu   = 24, // terminate process    cpu time limit exceeded (see setrlimit(2))
    SigXfsz   = 25, // terminate process    file size limit exceeded (see setrlimit(2))
    SigVtalrm = 26, // terminate process    virtual time alarm (see setitimer(2))
    SigProf   = 27, // terminate process    profiling timer alarm (see setitimer(2))
    SigWinch  = 28, // discard signal       Window size change
    SigInfo   = 29, // discard signal       status request from keyboard
    SigUsr1   = 30, // terminate process    User defined signal 1
    SigUsr2   = 31, // terminate process    User defined signal 2
  };
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
#ifdef _WIN32
  using Handle                               = void*;
  static constexpr Handle NIL_PROCESS_HANDLE = nullptr;
#else
  using Handle                               = intmax_t;
  static constexpr Handle NIL_PROCESS_HANDLE = -1;
#endif

  Handle    handle_      = NIL_PROCESS_HANDLE;
  StdioType stdin_type_  = Input::Inherit;
  StdioType stdout_type_ = Output::Inherit;
  StdioType stderr_type_ = Output::Inherit;

  Input  stdin_input_;
  Output stdout_output_;
  Output stderr_output_;

#ifdef _WIN32
  u32 group_;
#endif
};

} // namespace base

#endif
