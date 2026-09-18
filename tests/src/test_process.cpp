#ifndef __EMSCRIPTEN__

#include "base/containers/stack_array.hpp"
#include "base/containers/stack_storage.hpp"
#include "base/containers/string.hpp"
#include "base/os/file.hpp"
#include "base/os/process.hpp"
#include "base/ranges/slice.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"
#include "test_utils.hpp"
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _WIN32
#include <fileapi.h>
#include <windows.h>
#include <winerror.h>
#include <winnt.h>

#define TRY_SYSCALL_WIN32(syscall)                                             \
  do {                                                                         \
    if ((syscall) == FALSE) [[unlikely]] {                                     \
      base::panic(#syscall " failed: {}", GetLastError());                     \
    }                                                                          \
  } while (false)

#define O_CLOEXEC _O_NOINHERIT
#endif

#define TRY_SYSCALL(syscall)                                                   \
  [&]() {                                                                      \
    auto _result = syscall;                                                    \
    if (_result == -1) [[unlikely]] {                                          \
      perror(#syscall);                                                        \
      abort();                                                                 \
    }                                                                          \
    return _result;                                                            \
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
#ifdef _WIN32
  alignas(128) base::StackArray<HANDLE, 3> original = {};
  base::StackArray<HANDLE, 2> input                 = {};
  base::StackArray<HANDLE, 2> output                = {};
  base::StackArray<HANDLE, 2> error                 = {};
#else
  alignas(64) base::StackArray<int, 3> original;
  base::StackArray<int, 2> input  = {};
  base::StackArray<int, 2> output = {};
  base::StackArray<int, 2> error  = {};
#endif

  Pipes()
#ifndef _WIN32
      : original{
            fcntl(STDIN_FILENO, F_DUPFD_CLOEXEC, 0),
            fcntl(STDOUT_FILENO, F_DUPFD_CLOEXEC, 0),
            fcntl(STDERR_FILENO, F_DUPFD_CLOEXEC, 0),
        }
#endif
  {
#ifdef _WIN32
    TRY_SYSCALL_WIN32(CreatePipe(&input[0], &input[1], nullptr, 0));
    TRY_SYSCALL_WIN32(CreatePipe(&output[0], &output[1], nullptr, 0));
    TRY_SYSCALL_WIN32(CreatePipe(&error[0], &error[1], nullptr, 0));
    TRY_SYSCALL_WIN32(SetHandleInformation(input[0], HANDLE_FLAG_INHERIT,
                                           HANDLE_FLAG_INHERIT));
    TRY_SYSCALL_WIN32(SetHandleInformation(output[1], HANDLE_FLAG_INHERIT,
                                           HANDLE_FLAG_INHERIT));
    TRY_SYSCALL_WIN32(SetHandleInformation(error[1], HANDLE_FLAG_INHERIT,
                                           HANDLE_FLAG_INHERIT));

    // NOLINTBEGIN(performance-no-int-to-ptr)
    TRY_SYSCALL_WIN32(SetStdHandleEx(
        STD_INPUT_HANDLE, reinterpret_cast<HANDLE>(input[0]), &original[0]));
    TRY_SYSCALL_WIN32(SetStdHandleEx(
        STD_OUTPUT_HANDLE, reinterpret_cast<HANDLE>(output[1]), &original[1]));
    TRY_SYSCALL_WIN32(SetStdHandleEx(
        STD_ERROR_HANDLE, reinterpret_cast<HANDLE>(error[1]), &original[2]));

    //  NOLINTEND(performance-no-int-to-ptr)
#else
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
#endif
  }

  void restore_original_io() {
#ifdef _WIN32
    TRY_SYSCALL_WIN32(SetStdHandle(STD_INPUT_HANDLE, original[0]));
    TRY_SYSCALL_WIN32(SetStdHandle(STD_OUTPUT_HANDLE, original[1]));
    TRY_SYSCALL_WIN32(SetStdHandle(STD_ERROR_HANDLE, original[2]));

    TRY_SYSCALL_WIN32(CloseHandle(input[0]));
    TRY_SYSCALL_WIN32(CloseHandle(output[1]));
    TRY_SYSCALL_WIN32(CloseHandle(error[1]));
#else
    TRY_SYSCALL(::dup2(original[0], STDIN_FILENO));
    TRY_SYSCALL(::dup2(original[1], STDOUT_FILENO));
    TRY_SYSCALL(::dup2(original[2], STDERR_FILENO));
    TRY_SYSCALL(::close(original[0]));
    TRY_SYSCALL(::close(original[1]));
    TRY_SYSCALL(::close(original[2]));
#endif
  }
};
} // namespace

template <typename T>
static T create_io(Type type, char const* path, auto& pipe) noexcept {
  if (type == Type::File) {
    return T(path, base::File::Flags::Create);
  }
  if (type == Type::OtherPipe) {
    return pipe;
  }
  return static_cast<Process::StdioType>(type);
}

static void create_other_pipes(base::StackArray<Process, 3>& processes,
                               IoTypes                       types) {
  if (types.input == Type::OtherPipe) {
    Process::Stdio const stdio = {
        .input  = Process::Input::Inherit,
        .output = Process::Output::Pipe,
        .error  = Process::Output::Inherit,
    };
    processes[0] = Process::spawn({HELLO_WORLD_EXECUTABLE}, stdio).expect_ok();
  }
  Process::Stdio const stdio = {
      .input  = Process::Input::Pipe,
      .output = Process::Output::Pipe,
      .error  = Process::Output::Inherit,
  };
  if (types.output == Type::OtherPipe) {
    processes[1] = Process::spawn({CAT_EXECUTABLE}, stdio).expect_ok();
  }
  if (types.error == Type::OtherPipe) {
    processes[2] = Process::spawn({CAT_EXECUTABLE}, stdio).expect_ok();
  }
}

static constexpr char const* CHILD_STDIN_FILE_PATH =
    BASE_TESTS_DIR "/share/test_process/child_stdin.txt";
static constexpr char const* CHILD_STDOUT_FILE_PATH =
    BASE_TESTS_DIR "/share/test_process/child_stdout.txt";
static constexpr char const* CHILD_STDERR_FILE_PATH =
    BASE_TESTS_DIR "/share/test_process/child_stderr.txt";

static constexpr base::StringView INPUT_DATA = "hello world\n";

static void write_input(Type type, Process* proc, Pipes* pipes) {
  if (type == Type::Inherit) {
#ifdef _WIN32
    DWORD written = 0;
    TRY_SYSCALL_WIN32(WriteFile(pipes->input[1], INPUT_DATA.data(),
                                INPUT_DATA.size(), &written, nullptr));
#else
    ::write(pipes->input[1], INPUT_DATA.data(), INPUT_DATA.size());
#endif
  } else if (type == Type::Pipe) {
    proc->get_stdin().write(INPUT_DATA.bytes());
  }
}

static base::String get_pipe_output(Process::Output* output,
                                    base::StringView expected) {
  base::String str;
  str.resize(expected.size() + 256);
  base::Slice<base::uchar> bytes      = str.bytes();
  base::usize              total_read = output->read(bytes);
  while (total_read < expected.size()) {
    total_read +=
        output->read({bytes.data() + total_read, bytes.size() - total_read});
  }
  str.resize(total_read);
  return str;
}

static int check_file(char const* path, base::StringView expected) {
  int ret = 0;

  int const fildes =
      TRY_SYSCALL(open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0644));

  base::String buf;

  ssize_t result = ::read(fildes, buf.grow(64), 64);

  while (result != 0) {
    ASSERT(result != -1);
#ifdef _WIN32
    result = ::read(fildes, buf.grow(static_cast<base::usize>(result)),
                    static_cast<unsigned int>(result));
#else
    result = ::read(fildes, buf.grow(static_cast<base::usize>(result)),
                    static_cast<size_t>(result));
#endif
  }
  EXPECT_EQ(result, 0);

  buf.resize(buf.size() - 64);
  EXPECT_SVEQ(buf.view(), expected);

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
                                   IoTypes types) noexcept {
  int ret = 0;
  if (types.output == Type::OtherPipe) {
    Process::ExitStatus const status = processes[1].wait();
    EXPECT_SVEQ(status.unread_stdout, "arguments (to stdout):\n"
                                      "arg1\n"
                                      "arg2\n"
                                      "argument with spaces\n"
                                      "exiting (to stdout)\n");
  }
  if (types.error == Type::OtherPipe) {
    Process::ExitStatus const status = processes[2].wait();
    if (types.input == Type::Null) {
      EXPECT_SVEQ(status.unread_stdout, "stdin (to stderr):\n"
                                        "exiting (to stderr)\n");
    } else {
      EXPECT_SVEQ(status.unread_stdout, "stdin (to stderr):\n"
                                        "hello world\n"
                                        "exiting (to stderr)\n");
    }
  }
  return ret;
}

#ifdef _WIN32
static int check_fd_output(HANDLE file, base::StringView expected) {
#else
static int check_fd_output(int file, base::StringView expected) {
#endif
  int          ret = 0;
  base::String output;
#ifdef _WIN32
  DWORD   result  = 0;
  WINBOOL success = ReadFile(file, output.grow(512), 512, &result, nullptr);
  if (success == FALSE) [[unlikely]] {
    DWORD err = GetLastError();
    if (err != ERROR_BROKEN_PIPE) {
      base::panic("ReadFile() failed: {}", err);
    }
  }
#else
  ssize_t result = ::read(file, output.grow(512), 512);
#endif
#ifdef _WIN32
  while (result > 0) {
    success = ReadFile(file, output.grow(static_cast<base::usize>(result)), 512,
                       &result, nullptr);
    if (success == FALSE) [[unlikely]] {
      DWORD err = GetLastError();
      if (err != ERROR_BROKEN_PIPE) {
        base::panic("ReadFile() failed: {}", err);
      }
    }
  }
#else
  while (result > 0) {
    result = ::read(file, output.grow(static_cast<base::usize>(result)), 512);
  }
  ASSERT(result != -1);
#endif
  output.resize(output.size() - 512);
  EXPECT_SVEQ(output.view(), expected);
  return ret;
}

static int check_inherited_output(IoTypes types, Pipes const& pipes) {
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

namespace {
struct RuntimeOutput {
  base::StringView out;
  base::StringView err;
};
} // namespace

static int check_piped_output(IoTypes types, RuntimeOutput runtime_out,
                              Process::ExitStatus const& status) {
  int ret = 0;
  if (types.output == Type::Pipe) {
    if (types.input == Type::Pipe || types.input == Type::Inherit) {
      EXPECT_SVEQ(runtime_out.out, "arguments (to stdout):\n"
                                   "arg1\n"
                                   "arg2\n"
                                   "argument with spaces\n");
      EXPECT_SVEQ(status.unread_stdout, "exiting (to stdout)\n");
    } else {
      EXPECT_SVEQ(runtime_out.out, "arguments (to stdout):\n"
                                   "arg1\n"
                                   "arg2\n"
                                   "argument with spaces\n"
                                   "exiting (to stdout)\n");
      EXPECT_SVEQ(status.unread_stdout, "");
    }
  } else {
    EXPECT_SVEQ(status.unread_stdout, "");
  }
  if (types.error == Type::Pipe) {
    if (types.input == Type::Pipe || types.input == Type::Inherit) {
      EXPECT_SVEQ(runtime_out.err, "stdin (to stderr):\n"
                                   "hello world\n");
      EXPECT_SVEQ(status.unread_stderr, "exiting (to stderr)\n");
    } else if (types.input == Type::Null) {
      EXPECT_SVEQ(runtime_out.err, "stdin (to stderr):\n"
                                   "exiting (to stderr)\n");
      EXPECT_SVEQ(status.unread_stderr, "");
    } else {
      EXPECT_SVEQ(runtime_out.err, "stdin (to stderr):\n"
                                   "hello world\n"
                                   "exiting (to stderr)\n");
      EXPECT_SVEQ(status.unread_stderr, "");
    }
  } else {
    EXPECT_SVEQ(status.unread_stderr, "");
  }

  return ret;
}

static int test_run_c(IoTypes types) {
  int ret = 0;

  Pipes pipes;

  base::StackArray<Process, 3> other_pipes;
  create_other_pipes(other_pipes, types);

  Process::Stdio const stdio = {
      .input = create_io<Process::Input::Init>(
          types.input, CHILD_STDIN_FILE_PATH, other_pipes[0].get_stdout()),
      .output = create_io<Process::Output::Init>(
          types.output, CHILD_STDOUT_FILE_PATH, other_pipes[1].get_stdin()),
      .error = create_io<Process::Output::Init>(
          types.error, CHILD_STDERR_FILE_PATH, other_pipes[2].get_stdin()),
  };

  auto process = Process::spawn({STD_STREAMS_EXECUTABLE, "arg1", "arg2",
                                 "argument with spaces"},
                                stdio)
                     .expect_ok();

  write_input(types.input, &process, &pipes);

  base::String piped_output;
  if (types.output == Type::Pipe) {
    if (types.input == Type::Pipe || types.input == Type::Inherit) {
      piped_output =
          get_pipe_output(&process.get_stdout(), "arguments (to stdout):\n"
                                                 "arg1\n"
                                                 "arg2\n"
                                                 "argument with spaces\n");
    } else {
      piped_output =
          get_pipe_output(&process.get_stdout(), "arguments (to stdout):\n"
                                                 "arg1\n"
                                                 "arg2\n"
                                                 "argument with spaces\n"
                                                 "exiting (to stdout)\n");
    }
  }

  base::String piped_error;
  if (types.error == Type::Pipe) {
    if (types.input == Type::Pipe || types.input == Type::Inherit) {
      piped_error =
          get_pipe_output(&process.get_stderr(), "stdin (to stderr):\n"
                                                 "hello world\n");
    } else if (types.input == Type::Null) {
      piped_error =
          get_pipe_output(&process.get_stderr(), "stdin (to stderr):\n"
                                                 "exiting (to stderr)\n");
    } else {
      piped_error =
          get_pipe_output(&process.get_stderr(), "stdin (to stderr):\n"
                                                 "hello world\n"
                                                 "exiting (to stderr)\n");
    }
  }

#ifdef _WIN32
  TRY_SYSCALL_WIN32(CloseHandle(pipes.input[1]));
#else
  TRY_SYSCALL(::close(pipes.input[1]));
#endif
  Process::ExitStatus const status = process.wait();
  pipes.restore_original_io();

  EXPECT(check_file_output(types) == 0);
  EXPECT(check_other_pipe_output(other_pipes, types) == 0);
  EXPECT(check_inherited_output(types, pipes) == 0);
  EXPECT(check_piped_output(
             types, {.out = piped_output.view(), .err = piped_error.view()},
             status) == 0);

#ifdef _WIN32
  TRY_SYSCALL_WIN32(CloseHandle(pipes.output[0]));
  TRY_SYSCALL_WIN32(CloseHandle(pipes.error[0]));
#else
  TRY_SYSCALL(::close(pipes.output[0]));
  TRY_SYSCALL(::close(pipes.error[0]));
#endif

  return ret;
}

int main() {
  int ret = 0;

  int const file = ::open(CHILD_STDIN_FILE_PATH,
                          O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC, 0644);
  EXPECT(file != -1);
  [[maybe_unused]] ssize_t const written =
      ::write(file, INPUT_DATA.data(), INPUT_DATA.size());
  EXPECT(written == INPUT_DATA.size());
  EXPECT(close(file) == 0);

  (void)base::Logger::get_default_instance();

  base::StackArray<Type, 5> const types = {
      Type::Null, Type::Inherit, Type::Pipe, Type::File, Type::OtherPipe,
  };
  for (Type stdin_type : types) {
    for (Type stdout_type : types) {
      for (Type stderr_type : types) {
        base::log_info("calling test_run_c() with input type {}, output type "
                       "{} and error type {}",
                       stdin_type, stdout_type, stderr_type);
        base::Logger::get_default_instance().flush();
        if (test_run_c({.input  = stdin_type,
                        .output = stdout_type,
                        .error  = stderr_type}) != 0) {
          base::log_error("test_run_c() failed with input type {}, output type "
                          "{} and error type {}",
                          stdin_type, stdout_type, stderr_type);
          ret = 1;
        }
      }
    }
  }

  EXPECT(remove(CHILD_STDIN_FILE_PATH) == 0);

  base::Logger::get_default_instance().flush();

  return ret;
}

#else

#include "base/log/logger.hpp"

int main() {
  base::log_info("test_process does nothing when compiling with emscripten.");
}

#endif
