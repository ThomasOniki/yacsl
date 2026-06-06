#ifdef BASE_PLATFORM_WINDOWS

#include "base/containers/error.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/panic.hpp"
#include "base/sys/process.hpp"
#include "base/utilities/move_forward.hpp"
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
#include <synchapi.h>
#include <windows.h>
#include <winerror.h>

namespace base {

// NOLINTBEGIN(readability-make-member-function-const, cppcoreguidelines-pro-type-union-access)
usize Process::Stdio::write(const BasicStringView<uchar>& data) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(
      WriteFile(buffer_info_.handle, data.data(), static_cast<DWORD>(data.size()), &ret, nullptr));
  return static_cast<usize>(ret);
}
usize Process::Stdio::read(uchar* buf, usize size) noexcept {
  DWORD ret = 0;
  BASE_TRY_SYSCALL(ReadFile(buffer_info_.handle, buf, static_cast<DWORD>(size), &ret, nullptr) ==
                   FALSE);
  return static_cast<usize>(ret);
}

void Process::Stdio::seek_begin(isize offset) noexcept {
  auto low  = static_cast<LONG>(offset);
  auto high = static_cast<LONG>(offset / Limits<LONG>::max);
  BASE_TRY_SYSCALL(
      SetFilePointer(buffer_info_.handle, low, high > 0 ? &high : nullptr, FILE_BEGIN));
}
void Process::Stdio::seek_end(isize offset) noexcept {
  auto low  = static_cast<LONG>(offset);
  auto high = static_cast<LONG>(offset / Limits<LONG>::max);
  BASE_TRY_SYSCALL(SetFilePointer(buffer_info_.handle, low, high > 0 ? &high : nullptr, FILE_END));
}
void Process::Stdio::seek_cursor(isize offset) noexcept {
  auto low  = static_cast<LONG>(offset);
  auto high = static_cast<LONG>(offset / Limits<LONG>::max);
  BASE_TRY_SYSCALL(
      SetFilePointer(buffer_info_.handle, low, high > 0 ? &high : nullptr, FILE_CURRENT));
}
// NOLINTEND(readability-make-member-function-const, cppcoreguidelines-pro-type-union-access)

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)

using Type = Process::Stdio::Type;
using enum Type;

static void pipe_child_stdin(HANDLE* child_handle, Type type, HANDLE* pipe) {
  if (type != Buffer && type != ParentStdout && type != ParentStderr) {
    return;
  }
  SECURITY_ATTRIBUTES security_attr{
      .nLength              = sizeof(SECURITY_ATTRIBUTES),
      .lpSecurityDescriptor = nullptr,
      .bInheritHandle       = TRUE,
  };
  BASE_TRY_SYSCALL(CreatePipe(&pipe[0], &pipe[1], &security_attr, 0));
  BASE_TRY_SYSCALL(SetHandleInformation(pipe[1], HANDLE_FLAG_INHERIT, 0));
  *child_handle = pipe[0];
}

static void pipe_child_stdout(HANDLE* child_handle, Type type, HANDLE* pipe) {
  if (type != Buffer && type != ParentStdin) {
    return;
  }
  SECURITY_ATTRIBUTES security_attr{
      .nLength              = sizeof(SECURITY_ATTRIBUTES),
      .lpSecurityDescriptor = nullptr,
      .bInheritHandle       = TRUE,
  };
  BASE_TRY_SYSCALL(CreatePipe(&pipe[0], &pipe[1], &security_attr, 0));
  BASE_TRY_SYSCALL(SetHandleInformation(pipe[0], HANDLE_FLAG_INHERIT, 0));
  *child_handle = pipe[1];
}

static void redirect_child_stdio_to_file(HANDLE* child_handle, const char* path,
                                         FileOpenFlags flags) noexcept {
  const Result<HANDLE, File::OpenError> result =
      open_file(path, flags | FileOpenFlags::DontCloseOnExec);
  if (result.is_err()) [[unlikely]] {
    switch (result.assert_err()) {
    case File::OpenError::FileNotFound:
      panic("Failed to open file for redirected stdio: FileNotFound.");
    default:
      break;
    }
  }
  *child_handle = result.assert_ok();
}

static void set_parent_stdio(STARTUPINFOA* info, StackArray<FileHandle, 3> parent_io,
                             Type stdin_type, Type stdout_type, Type stderr_type) {
  if (stdin_type == ParentStdin) {
    info->hStdInput = parent_io[0];
  }

  if (stdout_type == ParentStdout) {
    info->hStdOutput = parent_io[1];
  } else if (stdout_type == ParentStderr) {
    info->hStdOutput = parent_io[2];
  }

  if (stderr_type == ParentStdout) {
    info->hStdError = parent_io[1];
  } else if (stderr_type == ParentStderr) {
    info->hStdError = parent_io[2];
  }
}

void Process::handle_piped_stdio(void* voidp) noexcept {
  auto* info = static_cast<STARTUPINFOA*>(voidp);

  if (stdin_.type_ == ParentStdin || stdout_.type_ == ParentStdin || stderr_.type_ == ParentStdin) {
    original_io_[0] = GetStdHandle(STD_INPUT_HANDLE);
  }
  if (stdin_.type_ == ParentStdout || stdout_.type_ == ParentStdout ||
      stderr_.type_ == ParentStdout) {
    original_io_[1] = GetStdHandle(STD_OUTPUT_HANDLE);
  }
  if (stdin_.type_ == ParentStderr || stdout_.type_ == ParentStderr ||
      stderr_.type_ == ParentStderr) {
    original_io_[2] = GetStdHandle(STD_ERROR_HANDLE);
  }

  set_parent_stdio(info, original_io_, stdin_.type_, stdout_.type_, stderr_.type_);

  pipe_child_stdin(&info->hStdInput, stdin_.type_, stdin_pipe_.data());
  pipe_child_stdout(&info->hStdOutput, stdout_.type_, stdout_pipe_.data());

  if (stdout_.type_ == ParentStdin && stderr_.type_ == ParentStdin) {
    info->hStdError = info->hStdOutput;
  } else {
    pipe_child_stdout(&info->hStdError, stderr_.type_, stderr_pipe_.data());
  }

  if (stdin_.type_ == Buffer) {
    stdin_.buffer_info_.process = this;
    stdin_.buffer_info_.handle  = stdin_pipe_[1];
  }
  if (stdout_.type_ == Buffer) {
    stdout_.buffer_info_.process = this;
    stdout_.buffer_info_.handle  = stdout_pipe_[0];
  }
  if (stderr_.type_ == Buffer) {
    stderr_.buffer_info_.process = this;
    stderr_.buffer_info_.handle  = stderr_pipe_[0];
  }

  if (stdin_.type_ == Pipe) {
    const Stdio*   pipe          = stdin_.pipe_;
    const Process* piped_process = pipe->buffer_info_.process;
    if (pipe == &piped_process->stdout_) {
      info->hStdInput = piped_process->stdout_pipe_[0];
    } else {
      info->hStdInput = piped_process->stderr_pipe_[0];
    }
  }
}

void Process::handle_file_stdio(void* voidp) noexcept {
  auto* info = static_cast<STARTUPINFOA*>(voidp);
  if (stdin_.type_ == File) {
    redirect_child_stdio_to_file(&info->hStdInput, stdin_.file_info_.path,
                                 stdin_.file_info_.flags | FileOpenFlags::Read);
  }
  if (stdout_.type_ == File) {
    redirect_child_stdio_to_file(&info->hStdOutput, stdout_.file_info_.path,
                                 stdout_.file_info_.flags | FileOpenFlags::Write);
  }

  if (stdout_.type_ == File && stderr_.type_ == File) {
    const bool same_flags =
        (stdout_.file_info_.flags | FileOpenFlags::Write | FileOpenFlags::DontCloseOnExec) ==
        (stderr_.file_info_.flags | FileOpenFlags::Write | FileOpenFlags::DontCloseOnExec);
    const bool same_paths = strcmp(stdout_.file_info_.path, stderr_.file_info_.path) == 0;

    if (same_flags && same_paths) {
      info->hStdError = info->hStdOutput;
    } else {
      redirect_child_stdio_to_file(&info->hStdError, stderr_.file_info_.path,
                                   stderr_.file_info_.flags | FileOpenFlags::Write);
    }
  } else if (stderr_.type_ == File) {
    redirect_child_stdio_to_file(&info->hStdError, stderr_.file_info_.path,
                                 stderr_.file_info_.flags | FileOpenFlags::Write);
  }
}

static Vector<char, 64> make_command_line_string(const char* const* argv) {
  Vector<char, 64> str;

  for (const char* const* it = argv; *it != nullptr; ++it) {
    StringView arg = StringView(*it, strlen(*it));
    str.push_back('"');
    usize backslash_count = 0;

    for (char chr : arg) {
      if (chr == '"') {
        for (usize i = 0; i < backslash_count; ++i) {
          str.push_back('\\');
        }
        str.push_back('\\');
        str.push_back('"');
      } else {
        str.push_back(chr);
      }

      if (chr == '\\') {
        ++backslash_count;
      } else {
        backslash_count = 0;
      }
    }

    for (usize i = 0; i < backslash_count; ++i) {
      str.push_back('\\');
    }

    str.push_back('"');
    str.push_back(' ');
  }
  str.pop_back(); // pop trailing space
  str.push_back('\0');

  return str;
}

Error<Process::SpawnError> Process::start(const char* const* argv, Stdio&& stdin_opts,
                                          Stdio&& stdout_opts, Stdio&& stderr_opts) noexcept {
  debug_panic_if(stdout_opts.type_ == Pipe, "You should pipe stdout to stdin, not the reverse");
  debug_panic_if(stderr_opts.type_ == Pipe, "You should pipe stderr to stdin, not the reverse");

  if (handle_ != nullptr) [[unlikely]] {
    wait();
  }

  stdin_  = base::move(stdin_opts);
  stdout_ = base::move(stdout_opts);
  stderr_ = base::move(stderr_opts);

  STARTUPINFOA info{
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
      .hStdInput       = INVALID_HANDLE_VALUE,
      .hStdOutput      = INVALID_HANDLE_VALUE,
      .hStdError       = INVALID_HANDLE_VALUE,
  };

  handle_piped_stdio(static_cast<void*>(&info));
  handle_file_stdio(static_cast<void*>(&info));

  PROCESS_INFORMATION proc_info{};

  Error<SpawnError> ret = NIL;

  if (CreateProcessA(nullptr, make_command_line_string(argv).data(), nullptr, nullptr, FALSE,
                     CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &info, &proc_info) == TRUE) {
    handle_ = proc_info.hProcess;
    group_  = proc_info.dwProcessId;
  } else {
    handle_         = NIL_PROCESS_HANDLE;
    const DWORD err = GetLastError();
    switch (err) {
    case ERROR_FILE_NOT_FOUND:
      ret = SpawnError::FileNotFound;
      break;
    default:
      panic("CreateProcessA() failed: {}", win32_err_str(err));
    }
  }

  if (stdin_.type_ == ParentStdout) {
    BASE_TRY_SYSCALL(SetStdHandle(STD_OUTPUT_HANDLE, stdin_pipe_[1]));
  } else if (stdin_.type_ == ParentStderr) {
    BASE_TRY_SYSCALL(SetStdHandle(STD_ERROR_HANDLE, stdin_pipe_[1]));
  }
  if (stdout_.type_ == ParentStdin) {
    BASE_TRY_SYSCALL(SetStdHandle(STD_INPUT_HANDLE, stdout_pipe_[0]));
  }
  if (stderr_.type_ == ParentStdin && stdout_.type_ != ParentStdin) {
    BASE_TRY_SYSCALL(SetStdHandle(STD_INPUT_HANDLE, stderr_pipe_[0]));
  }

  if (stdin_.type_ == Buffer || stdin_.type_ == ParentStdout || stdin_.type_ == ParentStderr) {
    BASE_TRY_SYSCALL(CloseHandle(stdin_pipe_[0]));
  }
  if (stdout_.type_ == Buffer || stdout_.type_ == ParentStdin) {
    BASE_TRY_SYSCALL(CloseHandle(stdout_pipe_[1]));
  }
  if (stderr_.type_ == Buffer || (stderr_.type_ == ParentStdin && stdout_.type_ != ParentStdin)) {
    BASE_TRY_SYSCALL(CloseHandle(stderr_pipe_[1]));
  }

  return ret;
}

// NOLINTNEXTLINE(readability-make-member-function-const, misc-no-recursion)
Process::ExitStatus Process::wait() noexcept {
  ExitStatus status = {
      .exit_code = 0,
      .signal    = Signal{0},
      .stopped   = Signal{0},
  };
  if (handle_ == nullptr) {
    return status;
  }
  if (stdin_.type_ == Pipe) {
    stdin_.pipe_->buffer_info_.process->wait();
  }

  if (stdin_.type_ == Buffer) {
    BASE_TRY_SYSCALL(CloseHandle(stdin_pipe_[1]));
  } else if (stdin_.type_ == ParentStdout) {
    BASE_TRY_SYSCALL(SetStdHandle(STD_OUTPUT_HANDLE, original_io_[0]));
  } else if (stdin_.type_ == ParentStderr) {
    BASE_TRY_SYSCALL(SetStdHandle(STD_ERROR_HANDLE, original_io_[0]));
  }

  if (stdout_.type_ == Buffer) {
    BASE_TRY_SYSCALL(CloseHandle(stdout_pipe_[0]));
  } else if (stdout_.type_ == ParentStdin) {
    BASE_TRY_SYSCALL(SetStdHandle(STD_INPUT_HANDLE, original_io_[1]));
  }

  if (stderr_.type_ == Buffer || (stderr_.type_ == ParentStdin && stdout_.type_ != ParentStdin)) {
    BASE_TRY_SYSCALL(CloseHandle(stderr_pipe_[0]));
  } else if (stderr_.type_ == ParentStdin) {
    BASE_TRY_SYSCALL(SetStdHandle(STD_INPUT_HANDLE, original_io_[2]));
  }

  BASE_TRY_SYSCALL(WaitForSingleObject(handle_, INFINITE));

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

// NOLINTEND(cppcoreguidelines-pro-type-union-access)

} // namespace base

#endif
