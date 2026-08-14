#ifndef __EMSCRIPTEN__

#include "base/containers/stack_array.hpp"
#include "base/containers/stack_storage.hpp"
#include "base/os/common.hpp"
#include "base/os/process.hpp"
#include "base/ranges/string_view.hpp"
#include "test_utils.hpp"
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TRY_SYSCALL(syscall)                                                                       \
  [&]() {                                                                                          \
    auto result = syscall;                                                                         \
    if (result == -1) [[unlikely]] {                                                               \
      perror(#syscall);                                                                            \
      abort();                                                                                     \
    }                                                                                              \
    return result;                                                                                 \
  }()

using base::Process;
using Stdio = Process::Stdio;

namespace {
enum class Type : base::u8 {
  Null,
  Inherit,
  Pipe,
  File,
  OtherPipe,
};
struct IoTypes {
  alignas(4) Type input;
  Type output;
  Type error;
};
struct IoStorage {
  alignas(128) base::StackStorage<Stdio> input;
  base::StackStorage<Stdio> output;
  base::StackStorage<Stdio> error;
};
struct Pipes {
  alignas(64) base::StackArray<int, 3> original;
  base::StackArray<int, 2> input  = {};
  base::StackArray<int, 2> output = {};
  base::StackArray<int, 2> error  = {};

  Pipes()
      : original({fcntl(STDIN_FILENO, F_DUPFD_CLOEXEC, 0), fcntl(STDOUT_FILENO, F_DUPFD_CLOEXEC, 0),
                  fcntl(STDERR_FILENO, F_DUPFD_CLOEXEC, 0)}) {
    // NOLINTNEXTLINE(android-cloexec-pipe)
    TRY_SYSCALL(::pipe(input.data()));
    // NOLINTNEXTLINE(android-cloexec-pipe)
    TRY_SYSCALL(::pipe(output.data()));
    // NOLINTNEXTLINE(android-cloexec-pipe)
    TRY_SYSCALL(::pipe(error.data()));

    TRY_SYSCALL(::fcntl(input[1], F_SETFD, FD_CLOEXEC));
    TRY_SYSCALL(::fcntl(output[0], F_SETFD, FD_CLOEXEC));
    TRY_SYSCALL(::fcntl(error[0], F_SETFD, FD_CLOEXEC));

    TRY_SYSCALL(::dup2(input[0], STDIN_FILENO));
    TRY_SYSCALL(::dup2(output[1], STDOUT_FILENO));
    TRY_SYSCALL(::dup2(error[1], STDERR_FILENO));
    TRY_SYSCALL(::close(input[0]));
    TRY_SYSCALL(::close(output[1]));
    TRY_SYSCALL(::close(error[1]));
  }

  void restore_original_io() {
    TRY_SYSCALL(::dup2(original[0], STDIN_FILENO));
    TRY_SYSCALL(::dup2(original[1], STDOUT_FILENO));
    TRY_SYSCALL(::dup2(original[2], STDERR_FILENO));
    TRY_SYSCALL(::close(original[0]));
    TRY_SYSCALL(::close(original[1]));
    TRY_SYSCALL(::close(original[2]));
  }
};
} // namespace

template <typename T> static T create_io(Type type, const char* path, auto& pipe) noexcept {
  if (type == Type::File) {
    return T(path, base::FileOpenFlags::Create);
  }
  if (type == Type::OtherPipe) {
    return pipe;
  }
  return static_cast<Process::StdioType>(type);
}

static void create_other_pipes(base::StackArray<Process, 3>& processes, IoTypes types) {
  if (types.input == Type::OtherPipe) {
    Process::Stdio stdio{
        .input  = Process::Input::Inherit,
        .output = Process::Output::Pipe,
        .error  = Process::Output::Inherit,
    };
    processes[0].start({HELLO_WORLD_EXECUTABLE}, stdio).expect_nil();
  }
  Process::Stdio stdio{
      .input  = Process::Input::Pipe,
      .output = Process::Output::Pipe,
      .error  = Process::Output::Inherit,
  };
  if (types.output == Type::OtherPipe) {
    processes[1].start({CAT_EXECUTABLE}, stdio).expect_nil();
  }
  if (types.error == Type::OtherPipe) {
    processes[2].start({CAT_EXECUTABLE}, stdio).expect_nil();
  }
}

static constexpr const char* CHILD_STDIN_FILE_PATH =
    BASE_TESTS_DIR "/share/test_process/child_stdin.txt";
static constexpr const char* CHILD_STDOUT_FILE_PATH =
    BASE_TESTS_DIR "/share/test_process/child_stdout.txt";
static constexpr const char* CHILD_STDERR_FILE_PATH =
    BASE_TESTS_DIR "/share/test_process/child_stderr.txt";

static constexpr base::StringView INPUT = "hello world\n";

static void write_input(Type type, Process* proc, Pipes* pipes, int* filedes) {
  if (type == Type::Inherit) {
    assert(::write(pipes->input[1], INPUT.data(), INPUT.size()) == INPUT.size());
  } else if (type == Type::File) {
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
    *filedes = ::open(CHILD_STDIN_FILE_PATH, O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
    assert(::write(*filedes, INPUT.data(), INPUT.size()) == INPUT.size());
    assert(*filedes != -1);
  } else if (type == Type::Pipe) {
    proc->get_stdin().write(INPUT.bytes());
  }
}

static base::String get_pipe_output(Process::Output* output, base::StringView expected) {
  base::String str;
  str.resize(expected.size() + 256);
  base::Slice<base::uchar> bytes      = str.bytes();
  base::usize              total_read = output->read(bytes);
  while (total_read < expected.size()) {
    total_read += output->read({bytes.data() + total_read, bytes.size() - total_read});
  }
  str.resize(total_read);
  return str;
}

static int check_file(const char* path, base::StringView expected) {
  int ret = 0;

  const int fildes = TRY_SYSCALL(open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0644));

  base::String buf;

  ssize_t result = ::read(fildes, buf.grow(64), 64);

  while (result != 0) {
    ASSERT(result != -1);
    result =
        ::read(fildes, buf.grow(static_cast<base::usize>(result)), static_cast<size_t>(result));
  }
  EXPECT_EQ(result, 0);

  buf.resize(buf.size() - 64);
  EXPECT_EQ(buf.view(), expected);

  TRY_SYSCALL(close(fildes));
  TRY_SYSCALL(remove(path));

  return ret;
}
static int check_file_output(IoTypes types) noexcept {
  int ret = 0;
  if (types.output == Type::File) {
    EXPECT(check_file(CHILD_STDOUT_FILE_PATH, "arguments (to stdout):\n"
                                              "arg1\n"
                                              "arg2\n"
                                              "argument with spaces\n"
                                              "exiting (to stdout)\n") == 0);
  }
  if (types.error == Type::File) {
    if (types.input == Type::Null) {
      EXPECT(check_file(CHILD_STDERR_FILE_PATH, "stdin (to stderr):\n"
                                                "exiting (to stderr)\n") == 0);
    } else {
      EXPECT(check_file(CHILD_STDERR_FILE_PATH, "stdin (to stderr):\n"
                                                "hello world\n"
                                                "exiting (to stderr)\n") == 0);
    }
  }
  return ret;
}

static int check_other_pipe_output(base::StackArray<Process, 3>& processes,
                                   IoTypes                       types) noexcept {
  int ret = 0;
  if (types.output == Type::OtherPipe) {
    Process::ExitStatus status = processes[1].wait();
    EXPECT_EQ(status.unread_stdout, "arguments (to stdout):\n"
                                    "arg1\n"
                                    "arg2\n"
                                    "argument with spaces\n"
                                    "exiting (to stdout)\n");
  }
  if (types.error == Type::OtherPipe) {
    Process::ExitStatus status = processes[2].wait();
    if (types.input == Type::Null) {
      EXPECT_EQ(status.unread_stdout, "stdin (to stderr):\n"
                                      "exiting (to stderr)\n");
    } else {
      EXPECT_EQ(status.unread_stdout, "stdin (to stderr):\n"
                                      "hello world\n"
                                      "exiting (to stderr)\n");
    }
  }
  return ret;
}

static int check_fd_output(int file, base::StringView expected) {
  int          ret = 0;
  base::String output;
  ssize_t      result = ::read(file, output.grow(512), 512);
  while (result > 0) {
    result = ::read(file, output.grow(static_cast<base::usize>(result)), 512);
  }
  ASSERT(result != -1);
  output.resize(output.size() - 512);
  EXPECT_EQ(output.view(), expected);
  return ret;
}

static int check_inherited_output(IoTypes types, const Pipes& pipes) {
  int ret = 0;
  if (types.output == Type::Inherit) {
    EXPECT(check_fd_output(pipes.output[0], "arguments (to stdout):\n"
                                            "arg1\n"
                                            "arg2\n"
                                            "argument with spaces\n"
                                            "exiting (to stdout)\n") == 0);
  }
  if (types.error == Type::Inherit) {
    if (types.input == Type::Null) {
      EXPECT(check_fd_output(pipes.error[0], "stdin (to stderr):\n"
                                             "exiting (to stderr)\n") == 0);
    } else {
      EXPECT(check_fd_output(pipes.error[0], "stdin (to stderr):\n"
                                             "hello world\n"
                                             "exiting (to stderr)\n") == 0);
    }
  }
  return ret;
}

static int check_piped_output(IoTypes types, base::StringView runtime_stdout,
                              base::StringView runtime_stderr, const Process::ExitStatus& status) {
  int ret = 0;
  if (types.output == Type::Pipe) {
    if (types.input == Type::Pipe || types.input == Type::Inherit) {
      EXPECT_EQ(runtime_stdout, "arguments (to stdout):\n"
                                "arg1\n"
                                "arg2\n"
                                "argument with spaces\n");
      EXPECT_EQ(status.unread_stdout, "exiting (to stdout)\n");
    } else {
      EXPECT_EQ(runtime_stdout, "arguments (to stdout):\n"
                                "arg1\n"
                                "arg2\n"
                                "argument with spaces\n"
                                "exiting (to stdout)\n");
      EXPECT_EQ(status.unread_stdout, "");
    }
  } else {
    EXPECT_EQ(status.unread_stdout, "");
  }
  if (types.error == Type::Pipe) {
    if (types.input == Type::Pipe || types.input == Type::Inherit) {
      EXPECT_EQ(runtime_stderr, "stdin (to stderr):\n"
                                "hello world\n");
      EXPECT_EQ(status.unread_stderr, "exiting (to stderr)\n");
    } else if (types.input == Type::Null) {
      EXPECT_EQ(runtime_stderr, "stdin (to stderr):\n"
                                "exiting (to stderr)\n");
      EXPECT_EQ(status.unread_stderr, "");
    } else {
      EXPECT_EQ(runtime_stderr, "stdin (to stderr):\n"
                                "hello world\n"
                                "exiting (to stderr)\n");
      EXPECT_EQ(status.unread_stderr, "");
    }
  } else {
    EXPECT_EQ(status.unread_stderr, "");
  }

  return ret;
}

static int test_run_c(IoTypes types) {
  int ret = 0;

  Pipes pipes;

  base::StackArray<Process, 3> other_pipes;
  create_other_pipes(other_pipes, types);
  Process::Stdio stdio = {
      .input  = create_io<Process::Input::Init>(types.input, CHILD_STDIN_FILE_PATH,
                                                other_pipes[0].get_stdout()),
      .output = create_io<Process::Output::Init>(types.output, CHILD_STDOUT_FILE_PATH,
                                                 other_pipes[1].get_stdin()),
      .error  = create_io<Process::Output::Init>(types.error, CHILD_STDERR_FILE_PATH,
                                                 other_pipes[2].get_stdin()),
  };

  Process process;
  process.start({STD_STREAMS_EXECUTABLE, "arg1", "arg2", "argument with spaces"}, stdio)
      .expect_nil();

  int input_fd = 0;
  write_input(types.input, &process, &pipes, &input_fd);

  base::String piped_output;
  if (types.output == Type::Pipe) {
    if (types.input == Type::Pipe || types.input == Type::Inherit) {
      piped_output = get_pipe_output(&process.get_stdout(), "arguments (to stdout):\n"
                                                            "arg1\n"
                                                            "arg2\n"
                                                            "argument with spaces\n");
    } else {
      piped_output = get_pipe_output(&process.get_stdout(), "arguments (to stdout):\n"
                                                            "arg1\n"
                                                            "arg2\n"
                                                            "argument with spaces\n"
                                                            "exiting (to stdout)\n");
    }
  }
  base::String piped_error;
  if (types.error == Type::Pipe) {
    if (types.input == Type::Pipe || types.input == Type::Inherit) {
      piped_error = get_pipe_output(&process.get_stderr(), "stdin (to stderr):\n"
                                                           "hello world\n");
    } else if (types.input == Type::Null) {
      piped_error = get_pipe_output(&process.get_stderr(), "stdin (to stderr):\n"
                                                           "exiting (to stderr)\n");
    } else {
      piped_error = get_pipe_output(&process.get_stderr(), "stdin (to stderr):\n"
                                                           "hello world\n"
                                                           "exiting (to stderr)\n");
    }
  }

  TRY_SYSCALL(::close(pipes.input[1]));
  Process::ExitStatus status = process.wait();
  pipes.restore_original_io();

  EXPECT(check_file_output(types) == 0);
  EXPECT(check_other_pipe_output(other_pipes, types) == 0);
  EXPECT(check_inherited_output(types, pipes) == 0);
  EXPECT(check_piped_output(types, piped_output.view(), piped_error.view(), status) == 0);

  TRY_SYSCALL(::close(pipes.output[0]));
  TRY_SYSCALL(::close(pipes.error[0]));

  return ret;
}

int main() {
  int ret = 0;

  (void)base::Logger::get_default_instance();

  base::StackArray<Type, 5> types = {Type::Null, Type::Inherit, Type::Pipe, Type::File,
                                     Type::OtherPipe};
  for (Type stdin_type : types) {
    for (Type stdout_type : types) {
      for (Type stderr_type : types) {
        base::log_info("calling test_run_c() with input type {}, output type {} and error type {}",
                       stdin_type, stdout_type, stderr_type);
        base::Logger::get_default_instance().flush();
        if (test_run_c({.input = stdin_type, .output = stdout_type, .error = stderr_type}) != 0) {
          base::log_error(
              "test_run_c() failed with input type {}, output type {} and error type {}",
              stdin_type, stdout_type, stderr_type);
          ret = 1;
        }
      }
    }
  }

  return ret;
}

#else

#include "base/logging/logger.hpp"

int main() { base::log_info("test_process does nothing when compiling with emscripten."); }

#endif
