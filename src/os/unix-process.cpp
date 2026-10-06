#if !defined(__EMSCRIPTEN__) && !defined(_WIN32)

#include "unix-utils.hpp"
#include "yacsl/containers/result.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/os/file.hpp"
#include "yacsl/os/process.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace yacsl::os {

using Type = Process::StdioType;
using enum Type;

static constexpr auto TYPE_FILE       = Type{254};
static constexpr auto TYPE_OTHER_PIPE = Type{255};

namespace {
struct State {
  alignas(64) posix_spawn_file_actions_t file_actions;
  Process::Stdio const*          stdio;
  containers::StackArray<int, 2> stdin_pipe;
  containers::StackArray<int, 2> stdout_pipe;
  containers::StackArray<int, 2> stderr_pipe;
};
} // namespace

#define CHECK_RETURN(func)                                                     \
  do {                                                                         \
    int const _result = func;                                                  \
    debug::assert_(_result == 0, #func " failed: {}", errno_str(_result));     \
  } while (false)

// NOLINTNEXTLINE(readability-make-member-function-const)
usize Process::Output::read(this Output&      self,
                            ranges::Slice<u8> buffer) noexcept {
  return static_cast<usize>(
      YACSL_TRY_SYSCALL(::read(self.handle_, buffer.data(), buffer.size())));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void Process::Input::write(this Input&             self,
                           ranges::Slice<u8 const> bytes) noexcept {
  usize written = static_cast<usize>(
      YACSL_TRY_SYSCALL(::write(self.handle_, bytes.data(), bytes.size())));
  while (written < bytes.size()) {
    written += static_cast<usize>(YACSL_TRY_SYSCALL(
        ::write(self.handle_, &bytes[written], bytes.size() - written)));
  }
}

static void redirect_child_stdio_to_file(posix_spawn_file_actions_t* actions,
                                         File::OpenFlags             flags,
                                         char const*                 path,
                                         int stream) noexcept {
  int const oflag =
      unix_flags(flags | File::OpenFlags::DontCloseOnExec |
                 (stream == STDIN_FILENO ? File::OpenFlags::Read
                                         : File::OpenFlags::Write));

  CHECK_RETURN(
      posix_spawn_file_actions_addopen(actions, stream, path, oflag, 0644));
}

void Process::open_files(this Process& self, void* pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);
  if (self.stdin_type_ == TYPE_FILE) {
    redirect_child_stdio_to_file(
        &state.file_actions, state.stdio->input.file_info_.flags,
        state.stdio->input.file_info_.path, STDIN_FILENO);
  } else if (self.stdin_type_ == Null) {
    redirect_child_stdio_to_file(&state.file_actions, File::OpenFlags::None,
                                 "/dev/null", STDIN_FILENO);
  }
  if (self.stdout_type_ == TYPE_FILE) {
    redirect_child_stdio_to_file(
        &state.file_actions, state.stdio->output.file_info_.flags,
        state.stdio->output.file_info_.path, STDOUT_FILENO);
  } else if (self.stdout_type_ == Null) {
    redirect_child_stdio_to_file(&state.file_actions, File::OpenFlags::None,
                                 "/dev/null", STDOUT_FILENO);
  }
  if (self.stderr_type_ == TYPE_FILE) {
    redirect_child_stdio_to_file(
        &state.file_actions, state.stdio->error.file_info_.flags,
        state.stdio->error.file_info_.path, STDERR_FILENO);
  } else if (self.stderr_type_ == Null) {
    redirect_child_stdio_to_file(&state.file_actions, File::OpenFlags::None,
                                 "/dev/null", STDERR_FILENO);
  }
}

void Process::create_pipes(this Process& self, void* pvoid_state) noexcept {
  // NOLINTBEGIN(android-cloexec-pipe)
  State& state = *static_cast<State*>(pvoid_state);
  if (self.stdin_type_ == Pipe) {
    YACSL_TRY_SYSCALL(::pipe(state.stdin_pipe.data()));
    self.stdin_input_.handle_ = state.stdin_pipe[1];
    CHECK_RETURN(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stdin_pipe[0], STDIN_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stdin_pipe[0]));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stdin_pipe[1]));
  } else if (self.stdin_type_ == TYPE_OTHER_PIPE) {
    CHECK_RETURN(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stdio->input.other_pipe_handle_,
        STDIN_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(
        &state.file_actions, state.stdio->input.other_pipe_handle_));
  }
  if (self.stdout_type_ == Pipe) {
    YACSL_TRY_SYSCALL(::pipe(state.stdout_pipe.data()));
    self.stdout_output_.handle_ = state.stdout_pipe[0];
    CHECK_RETURN(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stdout_pipe[1], STDOUT_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stdout_pipe[0]));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stdout_pipe[1]));
  } else if (self.stdout_type_ == TYPE_OTHER_PIPE) {
    CHECK_RETURN(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stdio->output.other_pipe_handle_,
        STDOUT_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(
        &state.file_actions, state.stdio->output.other_pipe_handle_));
  }
  if (self.stderr_type_ == Pipe) {
    YACSL_TRY_SYSCALL(::pipe(state.stderr_pipe.data()));
    self.stderr_output_.handle_ = state.stderr_pipe[0];
    CHECK_RETURN(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stderr_pipe[1], STDERR_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stderr_pipe[0]));
    CHECK_RETURN(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stderr_pipe[1]));
  } else if (self.stderr_type_ == TYPE_OTHER_PIPE) {
    CHECK_RETURN(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stdio->error.other_pipe_handle_,
        STDERR_FILENO));
    CHECK_RETURN(posix_spawn_file_actions_addclose(
        &state.file_actions, state.stdio->error.other_pipe_handle_));
  }
  // NOLINTEND(android-cloexec-pipe)
}

void Process::cleanup_after_spawn(this Process& self,
                                  void*         pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);
  CHECK_RETURN(posix_spawn_file_actions_destroy(&state.file_actions));

  if (self.stdin_type_ == Pipe) {
    YACSL_TRY_SYSCALL(close(state.stdin_pipe[0]));
  } else if (self.stdin_type_ == TYPE_OTHER_PIPE) {
    YACSL_TRY_SYSCALL(close(state.stdio->input.other_pipe_handle_));
  }
  if (self.stdout_type_ == Pipe) {
    YACSL_TRY_SYSCALL(close(state.stdout_pipe[1]));
  } else if (self.stdout_type_ == TYPE_OTHER_PIPE) {
    YACSL_TRY_SYSCALL(close(state.stdio->output.other_pipe_handle_));
  }
  if (self.stderr_type_ == Pipe) {
    YACSL_TRY_SYSCALL(close(state.stderr_pipe[1]));
  } else if (self.stderr_type_ == TYPE_OTHER_PIPE) {
    YACSL_TRY_SYSCALL(close(state.stdio->error.other_pipe_handle_));
  }
}

extern "C" char** environ;

containers::Result<Process, Process::SpawnError>
Process::spawn(char const* const* argv, Stdio const& stdio) noexcept {
  Process spawned;
  spawned.stdin_type_  = stdio.input.type_;
  spawned.stdout_type_ = stdio.output.type_;
  spawned.stderr_type_ = stdio.error.type_;

  State state = {
      .file_actions = {},
      .stdio        = &stdio,
      .stdin_pipe   = {-1, -1},
      .stdout_pipe  = {-1, -1},
      .stderr_pipe  = {-1, -1},
  };

  CHECK_RETURN(posix_spawn_file_actions_init(&state.file_actions));

  spawned.open_files(&state);
  spawned.create_pipes(&state);

  pid_t pid = 0;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  auto const* argv_mut = const_cast<char* const*>(argv);

  int const result = posix_spawnp(&pid, argv[0], &state.file_actions, nullptr,
                                  argv_mut, environ);
  if (result == ENOENT) {
    return SpawnError::FileNotFound;
  }
  debug::assert_(result == 0, "posix_spawnp failed: {}", errno_str(result));
  spawned.handle_ = pid;

  spawned.cleanup_after_spawn(&state);
  return spawned;
}

static containers::String drain_pipe(Process::Output* output) noexcept {
  containers::String data;
  ranges::Slice<u8>  bytes =
      ranges::Slice<char>(data.grow(PIPE_BUF), PIPE_BUF).bytes();
  usize result = output->read(bytes);
  while (result != 0) {
    bytes  = ranges::Slice<char>(data.grow(result), PIPE_BUF).bytes();
    result = output->read(bytes);
  }
  return data.resize(data.size() - PIPE_BUF);
}

void Process::cleanup_before_wait(this Process& self,
                                  ExitStatus*   status) noexcept {
  if (self.stdin_type_ == Pipe && self.stdin_input_.handle_ != -1) {
    YACSL_TRY_SYSCALL(close(self.stdin_input_.handle_));
    self.stdin_input_.handle_ = -1;
  }
  if (self.stdout_type_ == Pipe && self.stdout_output_.handle_ != -1) {
    status->unread_stdout = drain_pipe(&self.stdout_output_);
    YACSL_TRY_SYSCALL(close(self.stdout_output_.handle_));
    self.stdout_output_.handle_ = -1;
  }
  if (self.stderr_type_ == Pipe && self.stderr_output_.handle_ != -1) {
    status->unread_stderr = drain_pipe(&self.stderr_output_);
    YACSL_TRY_SYSCALL(close(self.stderr_output_.handle_));
    self.stderr_output_.handle_ = -1;
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
  if (self.handle_ == -1) {
    return status;
  }

  self.cleanup_before_wait(&status);

  int info = 0;
  YACSL_TRY_SYSCALL(waitpid(static_cast<pid_t>(self.handle_), &info, 0));

  self.handle_ = -1;

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
void Process::send_signal(this Process& self, Signal signal) noexcept {
  debug::assert_(self.handle_ == -1,
                 "Tried to send_signal to a Process that isn't running.");
  YACSL_TRY_SYSCALL(
      kill(static_cast<pid_t>(self.handle_), static_cast<int>(signal)));
  self.handle_ = -1;
}

} // namespace yacsl::os

#endif
