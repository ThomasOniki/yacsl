#ifdef _WIN32

#include "base/containers/string.hpp"
#include "base/logging/panic.hpp"
#include "base/os/process.hpp"
#include "base/ranges/slice.hpp"
#include "base/utilities/nil.hpp"
#include "base/utilities/type_aliases.hpp"
#include "win32-utils.hpp"
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

namespace base {

using Type = Process::StdioType;
using enum Type;

static constexpr auto TYPE_FILE       = Type{254};
static constexpr auto TYPE_OTHER_PIPE = Type{255};

namespace {
struct State {
  STARTUPINFOA          info;
  Process::Stdio const* stdio;
  StackArray<HANDLE, 2> stdin_pipe;
  StackArray<HANDLE, 2> stdout_pipe;
  StackArray<HANDLE, 2> stderr_pipe;
};
} // namespace

// NOLINTNEXTLINE(readability-make-member-function-const)
usize Process::Output::read(Slice<uchar> buffer) noexcept {
  DWORD   ret = 0;
  WINBOOL result =
      ReadFile(handle_, buffer.data(), static_cast<DWORD>(buffer.size()), &ret, nullptr);
  if (result == FALSE) [[unlikely]] {
    DWORD err = GetLastError();
    if (err == ERROR_BROKEN_PIPE) {
      return 0;
    }
    panic("ReadFile() failed: {}", win32_err_str(err));
  }
  return static_cast<usize>(ret);
}
// NOLINTNEXTLINE(readability-make-member-function-const)
void Process::Input::write(Slice<uchar const> bytes) noexcept {
  usize to_write = bytes.size();
  while (to_write > 0) {
    DWORD written = 0;
    BASE_TRY_SYSCALL(
        WriteFile(handle_, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr));
    to_write -= static_cast<usize>(written);
  }
}

void Process::open_files(void* pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);
  if (stdin_type_ == TYPE_FILE) {
    state.info.hStdInput = open_file(state.stdio->input.file_info_.path,
                                     state.stdio->input.file_info_.flags | File::Flags::Read |
                                         File::Flags::DontCloseOnExec)
                               .expect_ok()
                               .handle;
  } else if (stdin_type_ == Null) {
    state.info.hStdInput =
        open_file("NUL", File::Flags::Read | File::Flags::DontCloseOnExec).expect_ok().handle;
  }
  if (stdout_type_ == TYPE_FILE) {
    state.info.hStdOutput = open_file(state.stdio->output.file_info_.path,
                                      state.stdio->output.file_info_.flags | File::Flags::Write |
                                          File::Flags::DontCloseOnExec)
                                .expect_ok()
                                .handle;
  } else if (stdout_type_ == Null) {
    state.info.hStdOutput =
        open_file("NUL", File::Flags::Write | File::Flags::DontCloseOnExec).expect_ok().handle;
  }
  if (stderr_type_ == TYPE_FILE) {
    state.info.hStdError = open_file(state.stdio->error.file_info_.path,
                                     state.stdio->error.file_info_.flags | File::Flags::Write |
                                         File::Flags::DontCloseOnExec)
                               .expect_ok()
                               .handle;
  } else if (stderr_type_ == Null) {
    state.info.hStdError =
        open_file("NUL", File::Flags::Write | File::Flags::DontCloseOnExec).expect_ok().handle;
  }
}

static StackArray<HANDLE, 2> create_pipe() {
  StackArray<HANDLE, 2> handles = {};
  SECURITY_ATTRIBUTES   security_attr{
      .nLength              = sizeof(SECURITY_ATTRIBUTES),
      .lpSecurityDescriptor = nullptr,
      .bInheritHandle       = TRUE,
  };
  BASE_TRY_SYSCALL(CreatePipe(handles.data(), &handles[1], &security_attr, 0));
  return handles;
}

void Process::create_pipes(void* pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);

  if (stdin_type_ == Pipe) {
    state.stdin_pipe = create_pipe();
    BASE_TRY_SYSCALL(SetHandleInformation(state.stdin_pipe[1], HANDLE_FLAG_INHERIT, 0));
    state.info.hStdInput = state.stdin_pipe[0];
    stdin_input_.handle_ = state.stdin_pipe[1];
  } else if (stdin_type_ == TYPE_OTHER_PIPE) {
    state.info.hStdInput = state.stdio->input.other_pipe_handle_;
    BASE_TRY_SYSCALL(
        SetHandleInformation(state.info.hStdInput, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
  }
  if (stdout_type_ == Pipe) {
    state.stdout_pipe = create_pipe();
    BASE_TRY_SYSCALL(SetHandleInformation(state.stdout_pipe[0], HANDLE_FLAG_INHERIT, 0));
    state.info.hStdOutput  = state.stdout_pipe[1];
    stdout_output_.handle_ = state.stdout_pipe[0];
  } else if (stdout_type_ == TYPE_OTHER_PIPE) {
    state.info.hStdOutput = state.stdio->output.other_pipe_handle_;
    BASE_TRY_SYSCALL(
        SetHandleInformation(state.info.hStdOutput, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
  }
  if (stderr_type_ == Pipe) {
    state.stderr_pipe = create_pipe();
    BASE_TRY_SYSCALL(SetHandleInformation(state.stderr_pipe[0], HANDLE_FLAG_INHERIT, 0));
    state.info.hStdError   = state.stderr_pipe[1];
    stderr_output_.handle_ = state.stderr_pipe[0];
  } else if (stderr_type_ == TYPE_OTHER_PIPE) {
    state.info.hStdError = state.stdio->error.other_pipe_handle_;
    BASE_TRY_SYSCALL(
        SetHandleInformation(state.info.hStdError, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
  }
}

void Process::cleanup_after_spawn(void* pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);

  if (stdin_type_ == TYPE_FILE || stdin_type_ == Type::Null) {
    BASE_TRY_SYSCALL(CloseHandle(state.info.hStdInput));
  } else if (stdin_type_ == Pipe) {
    BASE_TRY_SYSCALL(CloseHandle(state.stdin_pipe[0]));
  } else if (stdin_type_ == TYPE_OTHER_PIPE) {
    BASE_TRY_SYSCALL(CloseHandle(state.stdio->input.other_pipe_handle_));
  }
  if (stdout_type_ == TYPE_FILE || stdout_type_ == Type::Null) {
    BASE_TRY_SYSCALL(CloseHandle(state.info.hStdOutput));
  } else if (stdout_type_ == Pipe) {
    BASE_TRY_SYSCALL(CloseHandle(state.stdout_pipe[1]));
  } else if (stdout_type_ == TYPE_OTHER_PIPE) {
    BASE_TRY_SYSCALL(CloseHandle(state.stdio->output.other_pipe_handle_));
  }
  if (stderr_type_ == TYPE_FILE || stderr_type_ == Type::Null) {
    BASE_TRY_SYSCALL(CloseHandle(state.info.hStdError));
  } else if (stderr_type_ == Pipe) {
    BASE_TRY_SYSCALL(CloseHandle(state.stderr_pipe[1]));
  } else if (stderr_type_ == TYPE_OTHER_PIPE) {
    BASE_TRY_SYSCALL(CloseHandle(state.stdio->error.other_pipe_handle_));
  }
}

static String make_command_line_string(char const* const* argv) {
  String str;

  for (char const* const* it = argv; *it != nullptr; ++it) {
    auto arg = StringView(*it, strlen(*it));
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

Error<Process::SpawnError> Process::start(char const* const* argv, Stdio const& stdio) noexcept {
  if (handle_ != nullptr) [[unlikely]] {
    wait();
  }

  stdin_type_  = stdio.input.type_;
  stdout_type_ = stdio.output.type_;
  stderr_type_ = stdio.error.type_;

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
              .hStdInput       = stdin_type_ == Inherit ? GetStdHandle(STD_INPUT_HANDLE) : nullptr,
              .hStdOutput = stdout_type_ == Inherit ? GetStdHandle(STD_OUTPUT_HANDLE) : nullptr,
              .hStdError  = stderr_type_ == Inherit ? GetStdHandle(STD_ERROR_HANDLE) : nullptr,
          },
      .stdio       = &stdio,
      .stdin_pipe  = {},
      .stdout_pipe = {},
      .stderr_pipe = {},
  };

  open_files(&state);
  create_pipes(&state);

  PROCESS_INFORMATION proc_info{};

  Error<SpawnError> ret = NIL;

  String cmdline = make_command_line_string(argv);

  if (CreateProcessA(nullptr, cmdline.c_str(), nullptr, nullptr, TRUE, CREATE_NEW_PROCESS_GROUP,
                     nullptr, nullptr, &state.info, &proc_info) == TRUE) [[likely]] {
    BASE_TRY_SYSCALL(CloseHandle(proc_info.hThread));
    handle_ = proc_info.hProcess;
    group_  = proc_info.dwProcessId;
  } else {
    handle_         = NIL_PROCESS_HANDLE;
    DWORD const err = GetLastError();
    switch (err) {
    case ERROR_FILE_NOT_FOUND:
      ret = SpawnError::FileNotFound;
      break;
    default:
      panic("CreateProcessA() failed: {}", win32_err_str(err));
    }
  }

  cleanup_after_spawn(&state);

  return ret;
}

static String drain_pipe(Process::Output* output) noexcept {
  static constexpr usize read_size = 1024;
  String                 data;
  Slice<uchar>           bytes  = Slice<char>(data.grow(read_size), read_size).bytes();
  usize                  result = output->read(bytes);
  while (result != 0) {
    bytes  = Slice<char>(data.grow(result), read_size).bytes();
    result = output->read(bytes);
  }
  return data.resize(data.size() - read_size);
}

void Process::cleanup_before_wait(ExitStatus* status) noexcept {
  if (stdin_type_ == Pipe && stdin_input_.handle_ != File::NIL_HANDLE) {
    BASE_TRY_SYSCALL(CloseHandle(stdin_input_.handle_));
    stdin_input_.handle_ = File::NIL_HANDLE;
  }
  if (stdout_type_ == Pipe && stdout_output_.handle_ != File::NIL_HANDLE) {
    status->unread_stdout = drain_pipe(&stdout_output_);
    BASE_TRY_SYSCALL(CloseHandle(stdout_output_.handle_));
    stdout_output_.handle_ = File::NIL_HANDLE;
  }
  if (stderr_type_ == Pipe && stderr_output_.handle_ != File::NIL_HANDLE) {
    status->unread_stderr = drain_pipe(&stderr_output_);
    BASE_TRY_SYSCALL(CloseHandle(stderr_output_.handle_));
    stderr_output_.handle_ = File::NIL_HANDLE;
  }
}

// NOLINTNEXTLINE(readability-make-member-function-const, misc-no-recursion)
Process::ExitStatus Process::wait() noexcept {
  ExitStatus status = {
      .unread_stdout = "",
      .unread_stderr = "",
      .exit_code     = 0,
      .signal        = Signal{0},
      .stopped       = Signal{0},
  };
  if (handle_ == nullptr) {
    return status;
  }

  cleanup_before_wait(&status);

  panic_if(WaitForSingleObject(handle_, INFINITE) != WAIT_OBJECT_0,
           "WaitForSingleObject failed: {}", win32_err_str());

  DWORD code = 0;
  BASE_TRY_SYSCALL(GetExitCodeProcess(handle_, &code));
  status.exit_code = static_cast<u32>(code);

  BASE_TRY_SYSCALL(CloseHandle(handle_));
  handle_ = NIL_PROCESS_HANDLE;

  return status;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void Process::send_signal(Signal signal) noexcept {
  if (signal == Signal::SigInt) {
    BASE_TRY_SYSCALL(GenerateConsoleCtrlEvent(CTRL_C_EVENT, group_));
  } else {
    BASE_TRY_SYSCALL(TerminateProcess(handle_, 1));
  }
  handle_ = nullptr;
}

} // namespace base

#endif
