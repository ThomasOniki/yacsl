#pragma once

#ifndef __EMSCRIPTEN__
#include "yacsl/compiler/attributes.hpp"
#include "yacsl/compiler/export.hpp"
#include "yacsl/containers/option.hpp"
#include "yacsl/containers/result.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/os/file.hpp"
#include "yacsl/ranges/initializer_list.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/nil.hpp"

namespace yacsl::os {

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

    constexpr StdioInit(char const* path, File::OpenFlags flags) noexcept
        : type_(TYPE_FILE), file_info_{.path = path, .flags = flags} {}
    constexpr StdioInit(ranges::StringView path, File::OpenFlags flags) noexcept
        : type_(TYPE_FILE),
          file_info_{
              .path =
                  containers::GenericString<char, 0,
                                            memory::MainArenaAllocator>(path)
                      .c_str(),
              .flags = flags,
          } {}

  private:
    friend class Process;

    StdioType type_ = StdioType::Null;
    struct FileInfo {
      alignas(16) char const* path = nullptr;
      File::OpenFlags flags        = File::OpenFlags::None;
    };
    union {
      FileInfo     file_info_;
      File::Handle other_pipe_handle_;
      utils::Nil   dummy_ = utils::NIL;
    };

    static constexpr auto TYPE_FILE       = StdioType{254};
    static constexpr auto TYPE_OTHER_PIPE = StdioType{255};
  };

public:
  class Input {
  public:
    void write(this Input&             self,
               ranges::Slice<u8 const> bytes) noexcept YACSL_API;

    ~Input() noexcept = default;

    using Init = StdioInit<true>;
    using enum StdioType;

  private:
    friend class Process;

    Input() noexcept                                          = default;
    Input(Input const&) noexcept                              = default;
    Input(Input&&) noexcept                                   = default;
    Input& operator=(this Input& self, Input const&) noexcept = default;
    Input& operator=(this Input& self, Input&&) noexcept      = default;

  private:
    File::Handle handle_ = File::NIL_HANDLE;
  };

  class Output {
  public:
    ~Output() noexcept = default;

    // returns 0 on EOF, between 1 and buffer.len() when sucessful.
    usize read(this Output& self, ranges::Slice<u8> buffer) noexcept YACSL_API;

    using Init = StdioInit<false>;
    using enum StdioType;

  private:
    friend class Process;

    Output() noexcept                                       = default;
    Output(Output const&) noexcept                          = default;
    Output(Output&&) noexcept                               = default;
    Output& operator=(this Output&, Output const&) noexcept = default;
    Output& operator=(this Output&, Output&&) noexcept      = default;

  private:
    File::Handle handle_ = File::NIL_HANDLE;
  };

  Process() noexcept = default;
  ~Process() noexcept { wait(); }

  Process(Process const&)                           = delete;
  Process& operator=(this Process&, Process const&) = delete;

  Process(Process&& other) noexcept
      : handle_(other.handle_), stdin_type_(other.stdin_type_),
        stdout_type_(other.stdout_type_), stderr_type_(other.stderr_type_),
        stdin_input_(memory::move(other.stdin_input_)),
        stdout_output_(memory::move(other.stdout_output_)),
        stderr_output_(memory::move(other.stderr_output_)) {
    other.handle_ = NIL_PROCESS_HANDLE;
  }

  Process& operator=(this Process& self YACSL_LIFETIMEBOUND,
                     Process&&          other) noexcept {
    debug::assert_(self.handle_ == NIL_PROCESS_HANDLE,
                   "Tried to memory::move assign an active Process instance.");
    self.handle_        = other.handle_;
    self.stdin_type_    = other.stdin_type_;
    self.stdout_type_   = other.stdout_type_;
    self.stderr_type_   = other.stderr_type_;
    self.stdin_input_   = memory::move(other.stdin_input_);
    self.stdout_output_ = memory::move(other.stdout_output_);
    self.stderr_output_ = memory::move(other.stderr_output_);
    other.handle_       = NIL_PROCESS_HANDLE;
    return self;
  }

  enum class SpawnError : u8 {
    FileNotFound = 1,
  };

  struct SpawnOpts {
    Input::Init        input  = Input::Null;
    Output::Init       output = Output::Null;
    Output::Init       error  = Output::Null;
    ranges::StringView cwd    = {nullptr, 0UZ};
  };

  [[nodiscard]] static containers::Result<Process, SpawnError>
  spawn(char const* const* argv, SpawnOpts const& opts) noexcept YACSL_API;

  [[nodiscard]] static containers::Result<Process, SpawnError>
  spawn(ranges::InitializerList<char const*> argv,
        SpawnOpts const&                     stdio) noexcept {
    containers::Vector<char const*, 6> cstrs;
    cstrs.reserve(argv.size());
    for (char const* str : argv) {
      cstrs.push(str);
    }
    cstrs.push(nullptr);
    return spawn(cstrs.data(), stdio);
  }

  [[nodiscard]] static containers::Result<Process, SpawnError>
  spawn(ranges::InitializerList<ranges::StringView> argv,
        SpawnOpts const&                            stdio) noexcept {
    return spawn<ranges::InitializerList<ranges::StringView>>(argv, stdio);
  }

  template <meta::RangeOf<ranges::StringView> Range>
  [[nodiscard]] static containers::Result<Process, SpawnError>
  spawn(Range const& argv, SpawnOpts const& stdio) noexcept {
    containers::Vector<containers::GenericString<char, 16>, 6> cstrs(argv);
    containers::Vector<char const*, 6>                         cstr_ptrs;
    cstr_ptrs.reserve(cstrs.len() + 1);
    for (auto& cstr : cstrs) {
      cstr_ptrs.push(cstr.c_str());
    }
    cstr_ptrs.push(nullptr);
    return spawn(cstr_ptrs.data(), stdio);
  }

  enum class Signal : u8 {
    SigHup  = 1,  // terminate process    terminal line hangup
    SigInt  = 2,  // terminate process    interrupt program
    SigQuit = 3,  // create core image    quit program
    SigIll  = 4,  // create core image    illegal instruction
    SigTrap = 5,  // create core image    trace trap
    SigAbrt = 6,  // create core image    abort program (formerly SIGIOT)
    SigEmt  = 7,  // create core image    emulate instruction executed
    SigFpe  = 8,  // create core image    floating-point exception
    SigKill = 9,  // terminate process    kill program
    SigBus  = 10, // create core image    bus error
    SigSegv = 11, // create core image    segmentation violation
    SigSys  = 12, // create core image    non-existent system call invoked
    SigPipe = 13, // terminate process    write on a pipe with no reader
    SigAlrm = 14, // terminate process    real-time timer expired
    SigTerm = 15, // terminate process    software termination signal
    SigUrg  = 16, // discard signal       urgent condition present on socket
    SigStop = 17, // stop process         stop (cannot be caught or ignored)
    SigTstp = 18, // stop process         stop signal generated from keyboard
    SigCont = 19, // discard signal       continue after stop
    SigChld = 20, // discard signal       child status has changed
    SigTtin = 21, // stop process         background read attempted from control
                  // terminal
    SigTtou = 22, // stop process         background write attempted to control
                  // terminal
    SigIo   = 23, // discard signal       I/O is possible on a descriptor (see
                  // fcntl(2))
    SigXcpu =
        24, // terminate process    cpu time limit exceeded (see setrlimit(2))
    SigXfsz =
        25, // terminate process    file size limit exceeded (see setrlimit(2))
    SigVtalrm =
        26, // terminate process    virtual time alarm (see setitimer(2))
    SigProf =
        27, // terminate process    profiling timer alarm (see setitimer(2))
    SigWinch = 28, // discard signal       Window size change
    SigInfo  = 29, // discard signal       status request from keyboard
    SigUsr1  = 30, // terminate process    User defined signal 1
    SigUsr2  = 31, // terminate process    User defined signal 2
  };
  struct ExitStatus {
    alignas(64) containers::String unread_stdout;
    containers::String unread_stderr;
    u32                exit_code;
    Signal             signal;
    Signal             stopped;
  };

  ExitStatus wait(this Process& self) noexcept YACSL_API;

  void send_signal(this Process& self, Signal signal) noexcept YACSL_API;

  template <typename Self>
  auto&& get_stdin(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).stdin_input_;
  }
  template <typename Self>
  auto&& get_stdout(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).stdout_output_;
  }
  template <typename Self>
  auto&& get_stderr(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).stderr_output_;
  }

private:
  void create_pipes(this Process& self, void* pvoid_state) noexcept;
  void open_files(this Process& self, void* pvoid_state) noexcept;
  void cleanup_after_spawn(this Process& self, void* pvoid_state) noexcept;
  void cleanup_before_wait(this Process& self, ExitStatus* status) noexcept;

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
  u32 group_ = 0;
#endif
};

} // namespace yacsl::os

#endif
