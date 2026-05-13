#include "base/sys/process.hpp"
#include "base/containers/stack_array.hpp"
#include "base/utilities/move_forward.hpp"
#include <assert.h>
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

static bool strip_flag(FileOpenFlags* flags, FileOpenFlags flag) {
	const FileOpenFlags stripped = *flags & ~flag;
	if (stripped != *flags) {
		*flags = stripped;
		return true;
	}
	return false;
}

static int unix_flags(FileOpenFlags flags) {
	unsigned int ret = 0;

	if (strip_flag(&flags, FileOpenFlags::Trunc)) {
		ret |= O_TRUNC;
	}
	if (strip_flag(&flags, FileOpenFlags::Create)) {
		ret |= O_CREAT;
	}
	if (strip_flag(&flags, FileOpenFlags::Append)) {
		ret |= O_APPEND;
	}
	if (!strip_flag(&flags, FileOpenFlags::DontCloseOnExec)) {
		ret |= O_CLOEXEC;
	}
	if (flags == FileOpenFlags::Read) {
		ret |= O_RDONLY;
	} else if (flags == FileOpenFlags::Write) {
		ret |= O_WRONLY;
	} else {
		ret |= O_RDWR;
	}
	return static_cast<int>(ret);
}

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access, android-cloexec-pipe, android-cloexec-dup)

using Type = Process::Stdio::Type;
using enum Type;

static void pipe_child_stdin(posix_spawn_file_actions_t* actions, Type type,
														 StackArray<int, 2>& pipe) {
	if (type != Buffer && type != ParentStdout && type != ParentStderr) {
		return;
	}
	assert(::pipe(pipe.data()) != -1);
	assert(posix_spawn_file_actions_adddup2(actions, pipe[0], STDIN_FILENO) == 0);
	assert(posix_spawn_file_actions_addclose(actions, pipe[0]) == 0);
	assert(posix_spawn_file_actions_addclose(actions, pipe[1]) == 0);
}

static void pipe_child_stdout(posix_spawn_file_actions_t* actions, Type type, int stream,
															StackArray<int, 2>& pipe) {
	if (type != Buffer && type != ParentStdin) {
		return;
	}

	assert(::pipe(pipe.data()) != -1);
	assert(posix_spawn_file_actions_adddup2(actions, pipe[1], stream) == 0);
	assert(posix_spawn_file_actions_addclose(actions, pipe[0]) == 0);
	assert(posix_spawn_file_actions_addclose(actions, pipe[1]) == 0);
}

static void redirect_child_stdio_to_file(posix_spawn_file_actions_t* actions, FileOpenFlags& flags,
																				 const char* path, int stream) noexcept {
	const int oflag =
			unix_flags(flags | FileOpenFlags::DontCloseOnExec |
								 (stream == STDIN_FILENO ? FileOpenFlags::Read : FileOpenFlags::Write));

	assert(posix_spawn_file_actions_addopen(actions, stream, path, oflag,
																					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == 0);
}

static void maybe_swap_child_stdout_and_err(posix_spawn_file_actions_t* actions, Type stdout_type,
																						Type stderr_type) {
	if (stdout_type == ParentStderr && stderr_type == ParentStdout) {
		assert(posix_spawn_file_actions_adddup2(actions, STDOUT_FILENO, 3) == 0);
		assert(posix_spawn_file_actions_adddup2(actions, STDERR_FILENO, STDOUT_FILENO) == 0);
		assert(posix_spawn_file_actions_adddup2(actions, 3, STDERR_FILENO) == 0);
		assert(posix_spawn_file_actions_addclose(actions, 3) == 0);
	} else if (stdout_type == ParentStderr) {
		assert(posix_spawn_file_actions_adddup2(actions, STDERR_FILENO, STDOUT_FILENO) == 0);
	} else if (stderr_type == ParentStdout) {
		assert(posix_spawn_file_actions_adddup2(actions, STDOUT_FILENO, STDERR_FILENO) == 0);
	}
}

void Process::handle_piped_stdio(void* voidp_file_actions) noexcept {
	auto* actions = static_cast<posix_spawn_file_actions_t*>(voidp_file_actions);

	maybe_swap_child_stdout_and_err(actions, stdout_.type_, stderr_.type_);

	pipe_child_stdin(actions, stdin_.type_, stdin_pipe_);
	pipe_child_stdout(actions, stdout_.type_, STDOUT_FILENO, stdout_pipe_);

	if (stdout_.type_ == ParentStdin && stderr_.type_ == ParentStdin) {
		assert(posix_spawn_file_actions_adddup2(actions, STDOUT_FILENO, STDERR_FILENO) == 0);
	} else {
		pipe_child_stdout(actions, stderr_.type_, STDERR_FILENO, stderr_pipe_);
	}

	if (stdout_.type_ == Buffer) {
		stdout_.process_ = this;
	}
	if (stderr_.type_ == Buffer) {
		stderr_.process_ = this;
	}

	if (stdin_.type_ == Pipe) {
		const Stdio*	 pipe					 = stdin_.pipe_;
		const Process* piped_process = pipe->process_;
		if (pipe == &piped_process->stdout_) {
			assert(posix_spawn_file_actions_adddup2(actions, piped_process->stdout_pipe_[0],
																							STDIN_FILENO) == 0);
			assert(posix_spawn_file_actions_addclose(actions, piped_process->stdout_pipe_[0]) == 0);
		} else {
			assert(pipe == &piped_process->stderr_);
			assert(posix_spawn_file_actions_adddup2(actions, piped_process->stderr_pipe_[0],
																							STDIN_FILENO) == 0);
			assert(posix_spawn_file_actions_addclose(actions, piped_process->stderr_pipe_[0]) == 0);
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
			assert(posix_spawn_file_actions_adddup2(actions, STDOUT_FILENO, STDERR_FILENO) == 0);
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

void Process::start(const char* const* argv, Stdio&& stdin_opts, Stdio&& stdout_opts,
										Stdio&& stderr_opts) noexcept {
	assert(stdout_opts.type_ != Pipe && "You should pipe stdout to stdin, not the reverse");
	assert(stderr_opts.type_ != Pipe && "You should pipe stderr to stdin, not the reverse");

	assert(handle_ == -1);

	stdin_	= base::move(stdin_opts);
	stdout_ = base::move(stdout_opts);
	stderr_ = base::move(stderr_opts);

	posix_spawn_file_actions_t actions{};
	assert(posix_spawn_file_actions_init(&actions) == 0);

	handle_piped_stdio(static_cast<void*>(&actions));
	handle_file_stdio(static_cast<void*>(&actions));

	pid_t pid = 0;

	const int result =
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
			posix_spawnp(&pid, argv[0], &actions, nullptr, const_cast<char* const*>(argv), environ);
	if (result != 0) {
		errno = result;
		(void)fputs("spawn ", stderr);
		perror(argv[0]);
		abort();
	}
	handle_ = pid;

	assert(posix_spawn_file_actions_destroy(&actions) == 0);

	if (stdin_.type_ == ParentStdout) {
		const int original = dup(STDOUT_FILENO);
		assert(original != -1);
		assert(dup2(stdin_pipe_[1], STDOUT_FILENO) != -1);
		assert(close(stdin_pipe_[1]) != -1);
		stdin_pipe_[1] = original;
	} else if (stdin_.type_ == ParentStderr) {
		const int original = dup(STDERR_FILENO);
		assert(original != -1);
		assert(dup2(stdin_pipe_[1], STDERR_FILENO) != -1);
		assert(close(stdin_pipe_[1]) != -1);
		stdin_pipe_[1] = original;
	}
	if (stdout_.type_ == ParentStdin) {
		const int original = dup(STDIN_FILENO);
		assert(original != -1);
		assert(dup2(stdout_pipe_[0], STDIN_FILENO) != -1);
		assert(close(stdout_pipe_[0]) != -1);
		stdout_pipe_[0] = original;
	}
	if (stderr_.type_ == ParentStdin && stdout_.type_ != ParentStdin) {
		const int original = dup(STDIN_FILENO);
		assert(original != -1);
		assert(dup2(stderr_pipe_[0], STDIN_FILENO) != -1);
		assert(close(stderr_pipe_[0]) != -1);
		stderr_pipe_[0] = original;
	}

	if (stdin_.type_ == Buffer || stdin_.type_ == ParentStdout || stdin_.type_ == ParentStderr) {
		assert(close(stdin_pipe_[0]) != -1);
	}
	if (stdout_.type_ == Buffer || stdout_.type_ == ParentStdin) {
		assert(close(stdout_pipe_[1]) != -1);
	}
	if (stderr_.type_ == Buffer || (stderr_.type_ == ParentStdin && stdout_.type_ != ParentStdin)) {
		assert(close(stderr_pipe_[1]) != -1);
	}
}

// NOLINTNEXTLINE(readability-make-member-function-const, misc-no-recursion)
Process::ExitStatus Process::wait() noexcept {
	ExitStatus status = {.exit_code = 0, .signal{0}, .stopped{0}};
	if (handle_ == -1) {
		return status;
	}
	if (stdin_.type_ == Pipe) {
		stdin_.pipe_->process_->wait();
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
		status.exit_code = WEXITSTATUS(info);
	}
	if (WIFSIGNALED(info)) {
		status.signal = Signal(WTERMSIG(info));
	}
	if (WIFSTOPPED(info)) {
		status.stopped = Signal(WSTOPSIG(info));
	}

	if (stdin_.type_ == Buffer || stdin_.type_ == ParentStdout || stdin_.type_ == ParentStderr) {
		if (stdin_.type_ == ParentStdout) {
			assert(dup2(stdin_pipe_[1], STDOUT_FILENO) == 0);
		} else if (stdin_.type_ == ParentStderr) {
			assert(dup2(stdin_pipe_[1], STDERR_FILENO) == 0);
		}
		assert(close(stdin_pipe_[1]) != -1);
	}

	if (stdout_.type_ == Buffer || stdout_.type_ == ParentStdin) {
		if (stdout_.type_ == ParentStdin) {
			assert(dup2(stdout_pipe_[0], STDIN_FILENO) == 0);
		}
		assert(close(stdout_pipe_[0]) != -1);
	}
	if (stderr_.type_ == Buffer || (stderr_.type_ == ParentStdin && stdout_.type_ != ParentStdin)) {
		if (stderr_.type_ == ParentStdin) {
			assert(dup2(stderr_pipe_[0], STDIN_FILENO) == 0);
		}
		assert(close(stderr_pipe_[0]) != -1);
	}

	return status;
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void Process::send_signal(Signal signal) noexcept {
	assert(handle_ != -1);
	kill(static_cast<pid_t>(handle_), static_cast<int>(signal));
	handle_ = -1;
}

Process::~Process() noexcept { wait(); }

// NOLINTEND(cppcoreguidelines-pro-type-union-access, android-cloexec-pipe, android-cloexec-dup)

} // namespace base
