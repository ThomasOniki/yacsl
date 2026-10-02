#ifdef _WIN32

#include "win32-utils.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/os/process.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/nil.hpp"
#include <consoleapi.h>
#include <consoleapi2.h>
#include <errhandlingapi.h>
#include <fileapi.h>
#include <handleapi.h>
#include <intsafe.h>
#include <processthreadsapi.h>
#include <string.h>
#include <synchapi.h>
#include <windows.h>
#include <winerror.h>
#include <winnt.h>

namespace yacsl::os {

using Type = Process::StdioType;
using enum Type;

static constexpr auto TYPE_FILE       = Type{254};
static constexpr auto TYPE_OTHER_PIPE = Type{255};

namespace {
struct State {
  STARTUPINFOA                      info;
  Process::Stdio const*             stdio;
  containers::StackArray<HANDLE, 2> stdin_pipe;
  containers::StackArray<HANDLE, 2> stdout_pipe;
  containers::StackArray<HANDLE, 2> stderr_pipe;
};
} // namespace

// NOLINTNEXTLINE(readability-make-member-function-const)
usize Process::Output::read(this Output&         self,
                            ranges::Slice<uchar> buffer) noexcept {
  DWORD   ret    = 0;
  WINBOOL result = ReadFile(self.handle_, buffer.data(),
                            static_cast<DWORD>(buffer.size()), &ret, nullptr);
  if (result == FALSE) [[unlikely]] {
    DWORD err = GetLastError();
    if (err == ERROR_BROKEN_PIPE) {
      return 0;
    }
    debug::panic("ReadFile() failed: {}", win32_err_str(err));
  }
  return static_cast<usize>(ret);
}
// NOLINTNEXTLINE(readability-make-member-function-const)
void Process::Input::write(this Input&                self,
                           ranges::Slice<uchar const> bytes) noexcept {
  usize to_write = bytes.size();
  while (to_write > 0) {
    DWORD written = 0;
    YACSL_TRY_SYSCALL(WriteFile(self.handle_, bytes.data(),
                                static_cast<DWORD>(bytes.size()), &written,
                                nullptr));
    to_write -= static_cast<usize>(written);
  }
}

void Process::open_files(this Process& self, void* pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);
  if (self.stdin_type_ == TYPE_FILE) {
    state.info.hStdInput =
        open_file_handle(state.stdio->input.file_info_.path,
                         state.stdio->input.file_info_.flags |
                             File::Flags::Read | File::Flags::DontCloseOnExec)
            .value();
  } else if (self.stdin_type_ == Null) {
    state.info.hStdInput =
        open_file_handle("NUL",
                         File::Flags::Read | File::Flags::DontCloseOnExec)
            .value();
  }
  if (self.stdout_type_ == TYPE_FILE) {
    state.info.hStdOutput =
        open_file_handle(state.stdio->output.file_info_.path,
                         state.stdio->output.file_info_.flags |
                             File::Flags::Write | File::Flags::DontCloseOnExec)
            .value();
  } else if (self.stdout_type_ == Null) {
    state.info.hStdOutput =
        open_file_handle("NUL",
                         File::Flags::Write | File::Flags::DontCloseOnExec)
            .value();
  }
  if (self.stderr_type_ == TYPE_FILE) {
    state.info.hStdError =
        open_file_handle(state.stdio->error.file_info_.path,
                         state.stdio->error.file_info_.flags |
                             File::Flags::Write | File::Flags::DontCloseOnExec)
            .value();
  } else if (self.stderr_type_ == Null) {
    state.info.hStdError =
        open_file_handle("NUL",
                         File::Flags::Write | File::Flags::DontCloseOnExec)
            .value();
  }
}

static containers::StackArray<HANDLE, 2> create_pipe() {
  containers::StackArray<HANDLE, 2> handles = {};
  SECURITY_ATTRIBUTES               security_attr{
      .nLength              = sizeof(SECURITY_ATTRIBUTES),
      .lpSecurityDescriptor = nullptr,
      .bInheritHandle       = TRUE,
  };
  YACSL_TRY_SYSCALL(CreatePipe(handles.data(), &handles[1], &security_attr, 0));
  return handles;
}

void Process::create_pipes(this Process& self, void* pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);

  if (self.stdin_type_ == Pipe) {
    state.stdin_pipe = create_pipe();
    YACSL_TRY_SYSCALL(
        SetHandleInformation(state.stdin_pipe[1], HANDLE_FLAG_INHERIT, 0));
    state.info.hStdInput      = state.stdin_pipe[0];
    self.stdin_input_.handle_ = state.stdin_pipe[1];
  } else if (self.stdin_type_ == TYPE_OTHER_PIPE) {
    state.info.hStdInput = state.stdio->input.other_pipe_handle_;
    YACSL_TRY_SYSCALL(SetHandleInformation(
        state.info.hStdInput, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
  }
  if (self.stdout_type_ == Pipe) {
    state.stdout_pipe = create_pipe();
    YACSL_TRY_SYSCALL(
        SetHandleInformation(state.stdout_pipe[0], HANDLE_FLAG_INHERIT, 0));
    state.info.hStdOutput       = state.stdout_pipe[1];
    self.stdout_output_.handle_ = state.stdout_pipe[0];
  } else if (self.stdout_type_ == TYPE_OTHER_PIPE) {
    state.info.hStdOutput = state.stdio->output.other_pipe_handle_;
    YACSL_TRY_SYSCALL(SetHandleInformation(
        state.info.hStdOutput, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
  }
  if (self.stderr_type_ == Pipe) {
    state.stderr_pipe = create_pipe();
    YACSL_TRY_SYSCALL(
        SetHandleInformation(state.stderr_pipe[0], HANDLE_FLAG_INHERIT, 0));
    state.info.hStdError        = state.stderr_pipe[1];
    self.stderr_output_.handle_ = state.stderr_pipe[0];
  } else if (self.stderr_type_ == TYPE_OTHER_PIPE) {
    state.info.hStdError = state.stdio->error.other_pipe_handle_;
    YACSL_TRY_SYSCALL(SetHandleInformation(
        state.info.hStdError, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
  }
}

void Process::cleanup_after_spawn(this Process& self,
                                  void*         pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);

  if (self.stdin_type_ == TYPE_FILE || self.stdin_type_ == Type::Null) {
    YACSL_TRY_SYSCALL(CloseHandle(state.info.hStdInput));
  } else if (self.stdin_type_ == Pipe) {
    YACSL_TRY_SYSCALL(CloseHandle(state.stdin_pipe[0]));
  } else if (self.stdin_type_ == TYPE_OTHER_PIPE) {
    YACSL_TRY_SYSCALL(CloseHandle(state.stdio->input.other_pipe_handle_));
  }
  if (self.stdout_type_ == TYPE_FILE || self.stdout_type_ == Type::Null) {
    YACSL_TRY_SYSCALL(CloseHandle(state.info.hStdOutput));
  } else if (self.stdout_type_ == Pipe) {
    YACSL_TRY_SYSCALL(CloseHandle(state.stdout_pipe[1]));
  } else if (self.stdout_type_ == TYPE_OTHER_PIPE) {
    YACSL_TRY_SYSCALL(CloseHandle(state.stdio->output.other_pipe_handle_));
  }
  if (self.stderr_type_ == TYPE_FILE || self.stderr_type_ == Type::Null) {
    YACSL_TRY_SYSCALL(CloseHandle(state.info.hStdError));
  } else if (self.stderr_type_ == Pipe) {
    YACSL_TRY_SYSCALL(CloseHandle(state.stderr_pipe[1]));
  } else if (self.stderr_type_ == TYPE_OTHER_PIPE) {
    YACSL_TRY_SYSCALL(CloseHandle(state.stdio->error.other_pipe_handle_));
  }
}

static containers::String make_command_line_string(char const* const* argv) {
  containers::String str;

  for (char const* const* it = argv; *it != nullptr; ++it) {
    auto arg = ranges::StringView(*it, strlen(*it));
    str += '"';
    usize backslash_count = 0;

    for (char chr : arg) {
      if (chr == '"') {
        for (usize i = 0; i < backslash_count; ++i) {
          str += '\\';
        }
        str += '\\';
        str += '"';
      } else {
        str += chr;
      }

      if (chr == '\\') {
        ++backslash_count;
      } else {
        backslash_count = 0;
      }
    }

    for (usize i = 0; i < backslash_count; ++i) {
      str += '\\';
    }

    str += "\" ";
  }
  str[str.size() - 1] = '\0';

  return str;
}

containers::Result<Process, Process::SpawnError>
Process::spawn(char const* const* argv, Stdio const& stdio) noexcept {
  Process spawned;
  spawned.stdin_type_  = stdio.input.type_;
  spawned.stdout_type_ = stdio.output.type_;
  spawned.stderr_type_ = stdio.error.type_;

  State state = {
      .info =
          {
              .cb              = sizeof(STARTUPINFOA),
              .lpReserved      = nullptr,
              .lpDesktop       = nullptr,
              .lpTitle         = nullptr,
              .dwX             = 0,
              .dwY             = 0,
              .dwXSize         = 0,
              .dwYSize         = 0,
              .dwXCountChars   = 0,
              .dwYCountChars   = 0,
              .dwFillAttribute = 0,
              .dwFlags         = STARTF_USESTDHANDLES,
              .wShowWindow     = 0,
              .cbReserved2     = 0,
              .lpReserved2     = nullptr,
              .hStdInput       = spawned.stdin_type_ == Inherit
                                     ? GetStdHandle(STD_INPUT_HANDLE)
                                     : nullptr,
              .hStdOutput      = spawned.stdout_type_ == Inherit
                                     ? GetStdHandle(STD_OUTPUT_HANDLE)
                                     : nullptr,
              .hStdError       = spawned.stderr_type_ == Inherit
                                     ? GetStdHandle(STD_ERROR_HANDLE)
                                     : nullptr,
          },
      .stdio       = &stdio,
      .stdin_pipe  = {},
      .stdout_pipe = {},
      .stderr_pipe = {},
  };

  spawned.open_files(&state);
  spawned.create_pipes(&state);

  PROCESS_INFORMATION proc_info{};

  containers::String cmdline = make_command_line_string(argv);

  if (CreateProcessA(nullptr, cmdline.c_str(), nullptr, nullptr, TRUE,
                     CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &state.info,
                     &proc_info) == FALSE) [[unlikely]] {
    DWORD const err = GetLastError();
    if (err == ERROR_FILE_NOT_FOUND) {
      return SpawnError::FileNotFound;
    }
    debug::panic("CreateProcessA() failed: {}", win32_err_str(err));
  }

  YACSL_TRY_SYSCALL(CloseHandle(proc_info.hThread));
  spawned.handle_ = proc_info.hProcess;
  spawned.group_  = proc_info.dwProcessId;

  spawned.cleanup_after_spawn(&state);
  return spawned;
}

static containers::String drain_pipe(Process::Output* output) noexcept {
  static constexpr usize read_size = 1024;
  containers::String     data;
  ranges::Slice<uchar>   bytes =
      ranges::Slice<char>(data.grow(read_size), read_size).bytes();
  usize result = output->read(bytes);
  while (result != 0) {
    bytes  = ranges::Slice<char>(data.grow(result), read_size).bytes();
    result = output->read(bytes);
  }
  return data.resize(data.size() - read_size);
}

void Process::cleanup_before_wait(this Process& self,
                                  ExitStatus*   status) noexcept {
  if (self.stdin_type_ == Pipe &&
      self.stdin_input_.handle_ != File::NIL_HANDLE) {
    YACSL_TRY_SYSCALL(CloseHandle(self.stdin_input_.handle_));
    self.stdin_input_.handle_ = File::NIL_HANDLE;
  }
  if (self.stdout_type_ == Pipe &&
      self.stdout_output_.handle_ != File::NIL_HANDLE) {
    status->unread_stdout = drain_pipe(&self.stdout_output_);
    YACSL_TRY_SYSCALL(CloseHandle(self.stdout_output_.handle_));
    self.stdout_output_.handle_ = File::NIL_HANDLE;
  }
  if (self.stderr_type_ == Pipe &&
      self.stderr_output_.handle_ != File::NIL_HANDLE) {
    status->unread_stderr = drain_pipe(&self.stderr_output_);
    YACSL_TRY_SYSCALL(CloseHandle(self.stderr_output_.handle_));
    self.stderr_output_.handle_ = File::NIL_HANDLE;
  }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
Process::ExitStatus Process::wait(this Process& self) noexcept {
  ExitStatus status = {
      .unread_stdout = "",
      .unread_stderr = "",
      .exit_code     = 0,
      .signal        = Signal{0},
      .stopped       = Signal{0},
  };
  if (self.handle_ == nullptr) {
    return status;
  }

  self.cleanup_before_wait(&status);

  debug::assert_(WaitForSingleObject(self.handle_, INFINITE) == WAIT_OBJECT_0,
                 "WaitForSingleObject failed: {}", win32_err_str());

  DWORD code = 0;
  YACSL_TRY_SYSCALL(GetExitCodeProcess(self.handle_, &code));
  status.exit_code = static_cast<u32>(code);

  YACSL_TRY_SYSCALL(CloseHandle(self.handle_));
  self.handle_ = NIL_PROCESS_HANDLE;

  return status;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void Process::send_signal(this Process& self, Signal signal) noexcept {
  if (signal == Signal::SigInt) {
    YACSL_TRY_SYSCALL(GenerateConsoleCtrlEvent(CTRL_C_EVENT, self.group_));
  } else {
    YACSL_TRY_SYSCALL(TerminateProcess(self.handle_, 1));
  }
  self.handle_ = nullptr;
}

} // namespace yacsl::os

#endif
