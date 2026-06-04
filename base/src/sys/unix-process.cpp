#ifndef BASE_PLATFORM_WINDOWS

#include "base/containers/error.hpp"
#include "base/containers/stack_array.hpp"
#include "base/logging/panic.hpp"
#include "base/sys/process.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/nil.hpp"
#include "unix-utils.hpp"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace base {

// NOLINTBEGIN(readability-make-member-function-const, cppcoreguidelines-pro-type-union-access)
usize Process::Stdio::write(const BasicStringView<uchar>& data) noexcept {
  assert_buffer("write");
  isize result = ::write(buffer_info_.handle, data.data(), data.size());
  panic_if(result == -1, "::write() failed: {}", errno_str());
  return static_cast<usize>(result);
}
usize Process::Stdio::read(uchar* buf, usize size) noexcept {
  assert_buffer("read");
  isize result = ::read(buffer_info_.handle, buf, size);
  panic_if(result == -1, "::read() failed: {}", errno_str());
  return static_cast<usize>(result);
}

void Process::Stdio::seek_begin(isize offset) noexcept {
  assert_buffer("seek_begin");
  off_t result = ::lseek(buffer_info_.handle, static_cast<off_t>(offset), SEEK_SET);
  panic_if(result == -1, "::lseek() failed: {}", errno_str());
  buffer_info_.cursor_pos = static_cast<usize>(result);
}
void Process::Stdio::seek_end(isize offset) noexcept {
  assert_buffer("seek_end");
  off_t result = ::lseek(buffer_info_.handle, static_cast<off_t>(offset), SEEK_END);
  panic_if(result == -1, "::lseek() failed: {}", errno_str());
  buffer_info_.cursor_pos = static_cast<usize>(result);
}
void Process::Stdio::seek_cursor(isize offset) noexcept {
  assert_buffer("seek_cursor");
  off_t result = ::lseek(buffer_info_.handle, static_cast<off_t>(offset), SEEK_CUR);
  panic_if(result == -1, "::lseek() failed: {}", errno_str());
  buffer_info_.cursor_pos = static_cast<usize>(result);
}

// NOLINTEND(readability-make-member-function-const, cppcoreguidelines-pro-type-union-access)

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access, android-cloexec-pipe, android-cloexec-dup)

using Type = Process::Stdio::Type;
using enum Type;

#define CHECK_RETURN(func)                                                                         \
  do {                                                                                             \
    const int _result = func;                                                                      \
    panic_if(_result != 0, #func " failed: {}", errno_str(_result));                               \
  } while (false)

static void pipe_child_stdin(posix_spawn_file_actions_t* actions, Type type,
                             StackArray<int, 2>& pipe) {
  if (type != Buffer && type != ParentStdout && type != ParentStderr) {
    return;
  }
  BASE_TRY_SYSCALL(::pipe(pipe.data()));
  CHECK_RETURN(posix_spawn_file_actions_adddup2(actions, pipe[0], STDIN_FILENO));
  CHECK_RETURN(posix_spawn_file_actions_addclose(actions, pipe[0]));
  CHECK_RETURN(posix_spawn_file_actions_addclose(actions, pipe[1]));
}

static void pipe_child_stdout(posix_spawn_file_actions_t* actions, Type type, int stream,
                              StackArray<int, 2>& pipe) {
  if (type != Buffer && type != ParentStdin) {
    return;
  }

  BASE_TRY_SYSCALL(::pipe(pipe.data()));
  CHECK_RETURN(posix_spawn_file_actions_adddup2(actions, pipe[1], stream));
  CHECK_RETURN(posix_spawn_file_actions_addclose(actions, pipe[0]));
  CHECK_RETURN(posix_spawn_file_actions_addclose(actions, pipe[1]));
}

static void redirect_child_stdio_to_file(posix_spawn_file_actions_t* actions, FileOpenFlags& flags,
                                         const char* path, int stream) noexcept {
  const int oflag =
      unix_flags(flags | FileOpenFlags::DontCloseOnExec |
                 (stream == STDIN_FILENO ? FileOpenFlags::Read : FileOpenFlags::Write));

  CHECK_RETURN(posix_spawn_file_actions_addopen(actions, stream, path, oflag,
                                                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
}

static void maybe_swap_child_stdout_and_err(posix_spawn_file_actions_t* actions, Type stdout_type,
                                            Type stderr_type) {
  if (stdout_type == ParentStderr && stderr_type == ParentStdout) {
    CHECK_RETURN(posix_spawn_file_actions_adddup2(actions, STDOUT_FILENO, 3));
    CHECK_RETURN(posix_spawn_file_actions_adddup2(actions, STDERR_FILENO, STDOUT_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_adddup2(actions, 3, STDERR_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(actions, 3));
  } else if (stdout_type == ParentStderr) {
    CHECK_RETURN(posix_spawn_file_actions_adddup2(actions, STDERR_FILENO, STDOUT_FILENO));
  } else if (stderr_type == ParentStdout) {
    CHECK_RETURN(posix_spawn_file_actions_adddup2(actions, STDOUT_FILENO, STDERR_FILENO));
  }
}

void Process::handle_piped_stdio(void* voidp_file_actions) noexcept {
  auto* actions = static_cast<posix_spawn_file_actions_t*>(voidp_file_actions);

  maybe_swap_child_stdout_and_err(actions, stdout_.type_, stderr_.type_);

  pipe_child_stdin(actions, stdin_.type_, stdin_pipe_);
  pipe_child_stdout(actions, stdout_.type_, STDOUT_FILENO, stdout_pipe_);

  if (stdout_.type_ == ParentStdin && stderr_.type_ == ParentStdin) {
    CHECK_RETURN(posix_spawn_file_actions_adddup2(actions, STDOUT_FILENO, STDERR_FILENO));
  } else {
    pipe_child_stdout(actions, stderr_.type_, STDERR_FILENO, stderr_pipe_);
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
      CHECK_RETURN(
          posix_spawn_file_actions_adddup2(actions, piped_process->stdout_pipe_[0], STDIN_FILENO));
      CHECK_RETURN(posix_spawn_file_actions_addclose(actions, piped_process->stdout_pipe_[0]));
    } else {
      CHECK_RETURN(posix_spawn_file_actions_adddup2(actions, piped_process->stderr_pipe_[0],
                                                    STDIN_FILENO) == 0);
      CHECK_RETURN(posix_spawn_file_actions_addclose(actions, piped_process->stderr_pipe_[0]));
    }
  }
}

void Process::handle_file_stdio(void* voidp_file_actions) noexcept {
  auto* actions = static_cast<posix_spawn_file_actions_t*>(voidp_file_actions);
  if (stdin_.type_ == File) {
    redirect_child_stdio_to_file(actions, stdin_.file_info_.flags, stdin_.file_info_.path,
                                 STDIN_FILENO);
  }
  if (stdout_.type_ == File) {
    redirect_child_stdio_to_file(actions, stdout_.file_info_.flags, stdout_.file_info_.path,
                                 STDOUT_FILENO);
  }

  if (stdout_.type_ == File && stderr_.type_ == File) {
    const bool same_flags =
        (stdout_.file_info_.flags | FileOpenFlags::Write | FileOpenFlags::DontCloseOnExec) ==
        (stderr_.file_info_.flags | FileOpenFlags::Write | FileOpenFlags::DontCloseOnExec);
    const bool same_paths = strcmp(stdout_.file_info_.path, stderr_.file_info_.path) == 0;

    if (same_flags && same_paths) {
      CHECK_RETURN(posix_spawn_file_actions_adddup2(actions, STDOUT_FILENO, STDERR_FILENO));
    } else {
      redirect_child_stdio_to_file(actions, stderr_.file_info_.flags, stderr_.file_info_.path,
                                   STDERR_FILENO);
    }
  } else if (stderr_.type_ == File) {
    redirect_child_stdio_to_file(actions, stderr_.file_info_.flags, stderr_.file_info_.path,
                                 STDERR_FILENO);
  }
}

extern "C" char** environ;

Error<Process::SpawnError> Process::start(const char* const* argv, Stdio&& stdin_opts,
                                          Stdio&& stdout_opts, Stdio&& stderr_opts) noexcept {
  debug_panic_if(stdout_opts.type_ == Pipe, "You should pipe stdout to stdin, not the reverse");
  debug_panic_if(stderr_opts.type_ == Pipe, "You should pipe stderr to stdin, not the reverse");

  if (handle_ != -1) [[unlikely]] {
    wait();
  }

  stdin_  = base::move(stdin_opts);
  stdout_ = base::move(stdout_opts);
  stderr_ = base::move(stderr_opts);

  posix_spawn_file_actions_t actions{};
  CHECK_RETURN(posix_spawn_file_actions_init(&actions));

  handle_piped_stdio(static_cast<void*>(&actions));
  handle_file_stdio(static_cast<void*>(&actions));

  pid_t pid = 0;

  const int result =
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
      posix_spawnp(&pid, argv[0], &actions, nullptr, const_cast<char* const*>(argv), environ);
  if (result != 0 && result != ENOENT) {
    panic("posix_spawnp failed: {}", errno_str(result));
  }
  handle_ = pid;

  CHECK_RETURN(posix_spawn_file_actions_destroy(&actions));

  if (stdin_.type_ == ParentStdout) {
    original_io_[1] = dup(STDOUT_FILENO);
    BASE_TRY_SYSCALL(original_io_[1]);
    BASE_TRY_SYSCALL(dup2(stdin_pipe_[1], STDOUT_FILENO));
    BASE_TRY_SYSCALL(close(stdin_pipe_[1]));
  } else if (stdin_.type_ == ParentStderr) {
    original_io_[2] = dup(STDERR_FILENO);
    BASE_TRY_SYSCALL(original_io_[2]);
    BASE_TRY_SYSCALL(dup2(stdin_pipe_[1], STDERR_FILENO));
    BASE_TRY_SYSCALL(close(stdin_pipe_[1]));
  }
  if (stdout_.type_ == ParentStdin) {
    original_io_[0] = dup(STDIN_FILENO);
    BASE_TRY_SYSCALL(original_io_[0]);
    BASE_TRY_SYSCALL(dup2(stdout_pipe_[0], STDIN_FILENO));
    BASE_TRY_SYSCALL(close(stdout_pipe_[0]));
  }
  if (stderr_.type_ == ParentStdin && stdout_.type_ != ParentStdin) {
    original_io_[0] = dup(STDIN_FILENO);
    BASE_TRY_SYSCALL(original_io_[0]);
    BASE_TRY_SYSCALL(dup2(stderr_pipe_[0], STDIN_FILENO));
    BASE_TRY_SYSCALL(close(stderr_pipe_[0]));
  }

  if (stdin_.type_ == Buffer || stdin_.type_ == ParentStdout || stdin_.type_ == ParentStderr) {
    BASE_TRY_SYSCALL(close(stdin_pipe_[0]));
  }
  if (stdout_.type_ == Buffer || stdout_.type_ == ParentStdin) {
    BASE_TRY_SYSCALL(close(stdout_pipe_[1]));
  }
  if (stderr_.type_ == Buffer || (stderr_.type_ == ParentStdin && stdout_.type_ != ParentStdin)) {
    BASE_TRY_SYSCALL(close(stderr_pipe_[1]));
  }

  return result == ENOENT ? Error(SpawnError::FileNotFound) : NIL;
}

// NOLINTNEXTLINE(readability-make-member-function-const, misc-no-recursion)
Process::ExitStatus Process::wait() noexcept {
  ExitStatus status = {
      .exit_code = 0,
      .signal    = Signal{0},
      .stopped   = Signal{0},
  };
  if (handle_ == -1) {
    return status;
  }
  if (stdin_.type_ == Pipe) {
    stdin_.pipe_->buffer_info_.process->wait();
  }

  if (stdin_.type_ == Buffer) {
    BASE_TRY_SYSCALL(close(stdin_pipe_[1]));
  } else if (stdin_.type_ == ParentStdout) {
    BASE_TRY_SYSCALL(dup2(original_io_[1], STDOUT_FILENO));
    BASE_TRY_SYSCALL(close(original_io_[1]));
  } else if (stdin_.type_ == ParentStderr) {
    BASE_TRY_SYSCALL(dup2(original_io_[2], STDERR_FILENO));
    BASE_TRY_SYSCALL(close(original_io_[2]));
  }

  if (stdout_.type_ == Buffer) {
    BASE_TRY_SYSCALL(close(stdout_pipe_[0]));
  } else if (stdout_.type_ == ParentStdin) {
    BASE_TRY_SYSCALL(dup2(original_io_[0], STDIN_FILENO));
    BASE_TRY_SYSCALL(close(original_io_[0]));
  }

  if (stderr_.type_ == Buffer) {
    BASE_TRY_SYSCALL(close(stderr_pipe_[0]));
  } else if (stderr_.type_ == ParentStdin && stdout_.type_ != ParentStdin) {
    BASE_TRY_SYSCALL(dup2(original_io_[0], STDIN_FILENO));
    BASE_TRY_SYSCALL(close(original_io_[0]));
  }

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
  kill(static_cast<pid_t>(handle_), static_cast<int>(signal));
  handle_ = -1;
}

// NOLINTEND(cppcoreguidelines-pro-type-union-access, android-cloexec-pipe, android-cloexec-dup)

} // namespace base

#endif
