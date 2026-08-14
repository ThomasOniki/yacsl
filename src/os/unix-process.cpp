#if !defined(__EMSCRIPTEN__) && !defined(_WIN32)
#include "base/containers/error.hpp"
#include "base/containers/stack_array.hpp"
#include "base/logging/panic.hpp"
#include "base/os/common.hpp"
#include "base/os/process.hpp"
#include "base/utilities/nil.hpp"
#include "base/utilities/type_aliases.hpp"
#include "try_syscall.hpp"
#include "utils.hpp"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace base {

using Type = Process::StdioType;
using enum Type;

static constexpr auto TYPE_FILE       = Type{254};
static constexpr auto TYPE_OTHER_PIPE = Type{255};

namespace {
struct State {
  alignas(64) posix_spawn_file_actions_t file_actions;
  const Process::Stdio* stdio;
  StackArray<int, 2>    stdin_pipe;
  StackArray<int, 2>    stdout_pipe;
  StackArray<int, 2>    stderr_pipe;
};
} // namespace

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access, android-cloexec-pipe)

#define CHECK_RETURN(func)                                                                         \
  do {                                                                                             \
    const int _result = func;                                                                      \
    panic_if(_result != 0, #func " failed: {}", errno_str(_result));                               \
  } while (false)

// NOLINTNEXTLINE(readability-make-member-function-const)
usize Process::Output::read(Slice<uchar> buffer) noexcept {
  ssize_t result = ::read(handle_, buffer.data(), buffer.size());
  panic_if(result == -1, "::read() failed: {}", errno_str());
  return static_cast<usize>(result);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void Process::Input::write(Slice<const uchar> bytes) noexcept {
  usize written = 0;
  if (bytes.size() >= PIPE_BUF) {
    while (written <= bytes.size() - PIPE_BUF) {
      ssize_t result = ::write(handle_, &bytes[written], PIPE_BUF);
      panic_if(result == -1, "::write() failed: {}", errno_str());
      written += static_cast<usize>(result);
    }
  }
  BASE_TRY_SYSCALL(::write(handle_, &bytes[written], bytes.size() - written));
}

static void redirect_child_stdio_to_file(posix_spawn_file_actions_t* actions, FileOpenFlags flags,
                                         const char* path, int stream) noexcept {
  const int oflag =
      unix_flags(flags | FileOpenFlags::DontCloseOnExec |
                 (stream == STDIN_FILENO ? FileOpenFlags::Read : FileOpenFlags::Write));

  CHECK_RETURN(posix_spawn_file_actions_addopen(actions, stream, path, oflag, 0644));
}

void Process::open_files(void* pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);
  // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
  if (stdin_type_ == TYPE_FILE) {
    redirect_child_stdio_to_file(&state.file_actions, state.stdio->input.file_info_.flags,
                                 static_cast<const char*>(state.stdio->input.file_info_.path),
                                 STDIN_FILENO);
  } else if (stdin_type_ == Null) {
    redirect_child_stdio_to_file(&state.file_actions, FileOpenFlags::None, "/dev/null",
                                 STDIN_FILENO);
  }
  if (stdout_type_ == TYPE_FILE) {
    redirect_child_stdio_to_file(&state.file_actions, state.stdio->output.file_info_.flags,
                                 static_cast<const char*>(state.stdio->output.file_info_.path),
                                 STDOUT_FILENO);
  } else if (stdout_type_ == Null) {
    redirect_child_stdio_to_file(&state.file_actions, FileOpenFlags::None, "/dev/null",
                                 STDOUT_FILENO);
  }
  if (stderr_type_ == TYPE_FILE) {
    redirect_child_stdio_to_file(&state.file_actions, state.stdio->error.file_info_.flags,
                                 static_cast<const char*>(state.stdio->error.file_info_.path),
                                 STDERR_FILENO);
  } else if (stderr_type_ == Null) {
    redirect_child_stdio_to_file(&state.file_actions, FileOpenFlags::None, "/dev/null",
                                 STDERR_FILENO);
  }
  // NOLINTEND(cppcoreguidelines-pro-type-union-access)
}

void Process::create_pipes(void* pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);
  // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
  if (stdin_type_ == Pipe) {
    // NOLINTNEXTLINE(android-cloexec-pipe)
    BASE_TRY_SYSCALL(::pipe(state.stdin_pipe.data()));
    stdin_input_.handle_ = state.stdin_pipe[1];
    CHECK_RETURN(
        posix_spawn_file_actions_adddup2(&state.file_actions, state.stdin_pipe[0], STDIN_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions, state.stdin_pipe[0]));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions, state.stdin_pipe[1]));
  } else if (stdin_type_ == TYPE_OTHER_PIPE) {
    CHECK_RETURN(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stdio->input.other_pipe_handle_, STDIN_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stdio->input.other_pipe_handle_));
  }
  if (stdout_type_ == Pipe) {
    // NOLINTNEXTLINE(android-cloexec-pipe)
    BASE_TRY_SYSCALL(::pipe(state.stdout_pipe.data()));
    stdout_output_.handle_ = state.stdout_pipe[0];
    CHECK_RETURN(
        posix_spawn_file_actions_adddup2(&state.file_actions, state.stdout_pipe[1], STDOUT_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions, state.stdout_pipe[0]));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions, state.stdout_pipe[1]));
  } else if (stdout_type_ == TYPE_OTHER_PIPE) {
    CHECK_RETURN(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stdio->output.other_pipe_handle_, STDOUT_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stdio->output.other_pipe_handle_));
  }

  if (stderr_type_ == Pipe) {
    // NOLINTNEXTLINE(android-cloexec-pipe)
    BASE_TRY_SYSCALL(::pipe(state.stderr_pipe.data()));
    stderr_output_.handle_ = state.stderr_pipe[0];
    CHECK_RETURN(
        posix_spawn_file_actions_adddup2(&state.file_actions, state.stderr_pipe[1], STDERR_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions, state.stderr_pipe[0]));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions, state.stderr_pipe[1]));
  } else if (stderr_type_ == TYPE_OTHER_PIPE) {
    CHECK_RETURN(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stdio->error.other_pipe_handle_, STDERR_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stdio->error.other_pipe_handle_));
  }
  // NOLINTEND(cppcoreguidelines-pro-type-union-access)
}

void Process::cleanup_after_spawn(void* pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);
  CHECK_RETURN(posix_spawn_file_actions_destroy(&state.file_actions));

  // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
  if (stdin_type_ == Pipe) {
    BASE_TRY_SYSCALL(close(state.stdin_pipe[0]));
  } else if (stdin_type_ == TYPE_OTHER_PIPE) {
    BASE_TRY_SYSCALL(close(state.stdio->input.other_pipe_handle_));
  }
  if (stdout_type_ == Pipe) {
    BASE_TRY_SYSCALL(close(state.stdout_pipe[1]));
  } else if (stdout_type_ == TYPE_OTHER_PIPE) {
    BASE_TRY_SYSCALL(close(state.stdio->output.other_pipe_handle_));
  }
  if (stderr_type_ == Pipe) {
    BASE_TRY_SYSCALL(close(state.stderr_pipe[1]));
  } else if (stderr_type_ == TYPE_OTHER_PIPE) {
    BASE_TRY_SYSCALL(close(state.stdio->error.other_pipe_handle_));
  }
  // NOLINTEND(cppcoreguidelines-pro-type-union-access)
}

static String drain_pipe(Process::Output* output) noexcept {
  String       data;
  Slice<uchar> bytes  = Slice<char>(data.grow(PIPE_BUF), PIPE_BUF).bytes();
  usize        result = output->read(bytes);
  while (result != 0) {
    bytes  = Slice<char>(data.grow(result), PIPE_BUF).bytes();
    result = output->read(bytes);
  }
  return data.resize(data.size() - PIPE_BUF);
}

void Process::cleanup_before_wait(ExitStatus* status) noexcept {
  if (stdin_type_ == Pipe && stdin_input_.handle_ != -1) {
    BASE_TRY_SYSCALL(close(stdin_input_.handle_));
    stdin_input_.handle_ = -1;
  } else if (stdin_type_ == TYPE_OTHER_PIPE) {
  }
  if (stdout_type_ == Pipe && stdout_output_.handle_ != -1) {
    status->unread_stdout = drain_pipe(&stdout_output_);
    BASE_TRY_SYSCALL(close(stdout_output_.handle_));
    stdout_output_.handle_ = -1;
  }
  if (stderr_type_ == Pipe && stderr_output_.handle_ != -1) {
    status->unread_stderr = drain_pipe(&stderr_output_);
    BASE_TRY_SYSCALL(close(stderr_output_.handle_));
    stderr_output_.handle_ = -1;
  }
}

extern "C" char** environ;

Error<Process::SpawnError> Process::start(const char* const* argv, const Stdio& stdio) noexcept {
  if (handle_ != -1) [[unlikely]] {
    wait();
  }

  stdin_type_  = stdio.input.type_;
  stdout_type_ = stdio.output.type_;
  stderr_type_ = stdio.error.type_;

  State state{
      .file_actions = {},
      .stdio        = &stdio,
      .stdin_pipe   = {-1, -1},
      .stdout_pipe  = {-1, -1},
      .stderr_pipe  = {-1, -1},
  };

  CHECK_RETURN(posix_spawn_file_actions_init(&state.file_actions));

  create_pipes(&state);
  open_files(&state);

  pid_t pid = 0;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  const auto* argv_mut = const_cast<char* const*>(argv);

  const int result = posix_spawnp(&pid, argv[0], &state.file_actions, nullptr, argv_mut, environ);
  if (result != 0 && result != ENOENT) {
    panic("posix_spawnp failed: {}", errno_str(result));
  }
  handle_ = pid;

  cleanup_after_spawn(&state);

  return result == ENOENT ? Error(SpawnError::FileNotFound) : NIL;
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
  if (handle_ == -1) {
    return status;
  }

  cleanup_before_wait(&status);

  int info = 0;
  for (;;) {
    if (waitpid(static_cast<pid_t>(handle_), &info, 0) != -1) {
      break;
    }
    if (errno == EINTR) {
      break;
    }
    perror("waitpid");
    abort();
  }

  handle_ = -1;

  if (WIFEXITED(info)) {
    status.exit_code = static_cast<u8>(WEXITSTATUS(info));
  }
  if (WIFSIGNALED(info)) {
    status.signal = Signal(WTERMSIG(info));
  }
  if (WIFSTOPPED(info)) {
    status.stopped = Signal(WSTOPSIG(info));
  }

  return status;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void Process::send_signal(Signal signal) noexcept {
  debug_panic_if(handle_ == -1, "Tried to send_signal to a Process that isn't running.");
  BASE_TRY_SYSCALL(kill(static_cast<pid_t>(handle_), static_cast<int>(signal)));
  handle_ = -1;
}

} // namespace base

#endif
