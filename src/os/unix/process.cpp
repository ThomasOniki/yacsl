#if !defined(__EMSCRIPTEN__) && !defined(_WIN32)

#include "yacsl/os/process.hpp"
#include "yacsl/containers/result.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/os/file.hpp"
#include "yacsl/os/unix/errno.hpp"
#include "yacsl/os/unix/oflag.hpp"
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
  Process::SpawnOpts const*      opts;
  containers::StackArray<int, 2> stdin_pipe;
  containers::StackArray<int, 2> stdout_pipe;
  containers::StackArray<int, 2> stderr_pipe;
};
} // namespace

static void check_return(int value, debug::SourceLocation location =
                                        debug::SourceLocation::current()) {
  debug::assert_(value == 0, "Posix function failed at {}:\n{}", location,
                 unix::errno_str(value));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
usize Process::Output::read(this Output&      self,
                            ranges::Slice<u8> buffer) noexcept {
  return static_cast<usize>(
      unix::try_unix(::read(self.handle_, buffer.data(), buffer.len())));
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void Process::Input::write(this Input&             self,
                           ranges::Slice<u8 const> bytes) noexcept {
  auto written = static_cast<usize>(
      unix::try_unix(::write(self.handle_, bytes.data(), bytes.len())));
  while (written < bytes.len()) {
    written += static_cast<usize>(unix::try_unix(
        ::write(self.handle_, &bytes[written], bytes.len() - written)));
  }
}

static void redirect_child_stdio_to_file(posix_spawn_file_actions_t* actions,
                                         File::OpenFlags             flags,
                                         char const*                 path,
                                         int stream) noexcept {
  int const oflag =
      unix::to_oflag(flags | File::OpenFlags::Inheritable |
                     (stream == STDIN_FILENO ? File::OpenFlags::Read
                                             : File::OpenFlags::Write));

  check_return(
      posix_spawn_file_actions_addopen(actions, stream, path, oflag, 0644));
}

void Process::open_files(this Process& self, void* pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);
  if (self.stdin_type_ == TYPE_FILE) {
    redirect_child_stdio_to_file(
        &state.file_actions, state.opts->input.file_info_.flags,
        state.opts->input.file_info_.path, STDIN_FILENO);
  } else if (self.stdin_type_ == Null) {
    redirect_child_stdio_to_file(&state.file_actions, File::OpenFlags::None,
                                 "/dev/null", STDIN_FILENO);
  }
  if (self.stdout_type_ == TYPE_FILE) {
    redirect_child_stdio_to_file(
        &state.file_actions, state.opts->output.file_info_.flags,
        state.opts->output.file_info_.path, STDOUT_FILENO);
  } else if (self.stdout_type_ == Null) {
    redirect_child_stdio_to_file(&state.file_actions, File::OpenFlags::None,
                                 "/dev/null", STDOUT_FILENO);
  }
  if (self.stderr_type_ == TYPE_FILE) {
    redirect_child_stdio_to_file(
        &state.file_actions, state.opts->error.file_info_.flags,
        state.opts->error.file_info_.path, STDERR_FILENO);
  } else if (self.stderr_type_ == Null) {
    redirect_child_stdio_to_file(&state.file_actions, File::OpenFlags::None,
                                 "/dev/null", STDERR_FILENO);
  }
}

void Process::create_pipes(this Process& self, void* pvoid_state) noexcept {
  // NOLINTBEGIN(android-cloexec-pipe)
  State& state = *static_cast<State*>(pvoid_state);
  if (self.stdin_type_ == Pipe) {
    unix::try_unix(::pipe(state.stdin_pipe.data()));
    self.stdin_input_.handle_ = state.stdin_pipe[1];
    check_return(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stdin_pipe[0], STDIN_FILENO));
    check_return(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stdin_pipe[0]));
    check_return(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stdin_pipe[1]));
  } else if (self.stdin_type_ == TYPE_OTHER_PIPE) {
    check_return(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.opts->input.other_pipe_handle_,
        STDIN_FILENO));
    check_return(posix_spawn_file_actions_addclose(
        &state.file_actions, state.opts->input.other_pipe_handle_));
  }
  if (self.stdout_type_ == Pipe) {
    unix::try_unix(::pipe(state.stdout_pipe.data()));
    self.stdout_output_.handle_ = state.stdout_pipe[0];
    check_return(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stdout_pipe[1], STDOUT_FILENO));
    check_return(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stdout_pipe[0]));
    check_return(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stdout_pipe[1]));
  } else if (self.stdout_type_ == TYPE_OTHER_PIPE) {
    check_return(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.opts->output.other_pipe_handle_,
        STDOUT_FILENO));
    check_return(posix_spawn_file_actions_addclose(
        &state.file_actions, state.opts->output.other_pipe_handle_));
  }
  if (self.stderr_type_ == Pipe) {
    unix::try_unix(::pipe(state.stderr_pipe.data()));
    self.stderr_output_.handle_ = state.stderr_pipe[0];
    check_return(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.stderr_pipe[1], STDERR_FILENO));
    check_return(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stderr_pipe[0]));
    check_return(posix_spawn_file_actions_addclose(&state.file_actions,
                                                   state.stderr_pipe[1]));
  } else if (self.stderr_type_ == TYPE_OTHER_PIPE) {
    check_return(posix_spawn_file_actions_adddup2(
        &state.file_actions, state.opts->error.other_pipe_handle_,
        STDERR_FILENO));
    check_return(posix_spawn_file_actions_addclose(
        &state.file_actions, state.opts->error.other_pipe_handle_));
  }
  // NOLINTEND(android-cloexec-pipe)
}

void Process::cleanup_after_spawn(this Process& self,
                                  void*         pvoid_state) noexcept {
  State& state = *static_cast<State*>(pvoid_state);
  check_return(posix_spawn_file_actions_destroy(&state.file_actions));

  if (self.stdin_type_ == Pipe) {
    unix::try_unix(close(state.stdin_pipe[0]));
  } else if (self.stdin_type_ == TYPE_OTHER_PIPE) {
    unix::try_unix(close(state.opts->input.other_pipe_handle_));
  }
  if (self.stdout_type_ == Pipe) {
    unix::try_unix(close(state.stdout_pipe[1]));
  } else if (self.stdout_type_ == TYPE_OTHER_PIPE) {
    unix::try_unix(close(state.opts->output.other_pipe_handle_));
  }
  if (self.stderr_type_ == Pipe) {
    unix::try_unix(close(state.stderr_pipe[1]));
  } else if (self.stderr_type_ == TYPE_OTHER_PIPE) {
    unix::try_unix(close(state.opts->error.other_pipe_handle_));
  }
}

extern "C" char** environ;

containers::Result<Process, Process::SpawnError>
Process::spawn(char const* const* argv, SpawnOpts const& opts) noexcept {
  Process spawned;
  spawned.stdin_type_  = opts.input.type_;
  spawned.stdout_type_ = opts.output.type_;
  spawned.stderr_type_ = opts.error.type_;

  State state = {
      .file_actions = {},
      .opts         = &opts,
      .stdin_pipe   = {-1, -1},
      .stdout_pipe  = {-1, -1},
      .stderr_pipe  = {-1, -1},
  };

  check_return(posix_spawn_file_actions_init(&state.file_actions));

  if (opts.cwd.data() != nullptr) {
    posix_spawn_file_actions_addchdir(&state.file_actions,
                                      containers::String(opts.cwd).c_str());
  }

  spawned.open_files(&state);
  spawned.create_pipes(&state);

  pid_t pid = 0;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  auto const* argv_mut = const_cast<char* const*>(argv);

  int const result = posix_spawnp(&pid, argv[0], &state.file_actions, nullptr,
                                  argv_mut, environ);
  if (result == ENOENT) {
    return SpawnError::NotFound;
  }
  debug::assert_(result == 0, "posix_spawnp failed: {}",
                 unix::errno_str(result));
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
  return data.resize(data.len() - PIPE_BUF);
}

void Process::cleanup_before_wait(this Process& self,
                                  ExitStatus*   status) noexcept {
  if (self.stdin_type_ == Pipe && self.stdin_input_.handle_ != -1) {
    unix::try_unix(close(self.stdin_input_.handle_));
    self.stdin_input_.handle_ = -1;
  }
  if (self.stdout_type_ == Pipe && self.stdout_output_.handle_ != -1) {
    status->unread_stdout = drain_pipe(&self.stdout_output_);
    unix::try_unix(close(self.stdout_output_.handle_));
    self.stdout_output_.handle_ = -1;
  }
  if (self.stderr_type_ == Pipe && self.stderr_output_.handle_ != -1) {
    status->unread_stderr = drain_pipe(&self.stderr_output_);
    unix::try_unix(close(self.stderr_output_.handle_));
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
  unix::try_unix(waitpid(static_cast<pid_t>(self.handle_), &info, 0));

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
  unix::try_unix(
      kill(static_cast<pid_t>(self.handle_), static_cast<int>(signal)));
  self.handle_ = -1;
}

} // namespace yacsl::os

#endif
