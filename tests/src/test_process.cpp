#ifndef __EMSCRIPTEN__

#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/stack_storage.hpp"
#include "yacsl/containers/string.hpp"
#include "yacsl/debug/assert.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/os/file.hpp"
#include "yacsl/os/process.hpp"
#include "yacsl/os/unix/errno.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/testing/expect.hpp"
#include "yacsl/testing/test.hpp"
#include "yacsl/type_aliases.hpp"
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _WIN32
#include "yacsl/os/windows/win_err.hpp"
#include <fileapi.h>
#include <windows.h>
#include <winerror.h>
#include <winnt.h>

#define O_CLOEXEC _O_NOINHERIT
#endif

using yacsl::os::Process;
using Stdio = Process::SpawnOpts;

namespace {
enum class Type : yacsl::u8 {
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
  alignas(128) yacsl::containers::StackStorage<Stdio> input;
  yacsl::containers::StackStorage<Stdio> output;
  yacsl::containers::StackStorage<Stdio> error;
};
struct Pipes {
#ifdef _WIN32
  alignas(128) yacsl::containers::StackArray<HANDLE, 3> original = {};
  yacsl::containers::StackArray<HANDLE, 2> input                 = {};
  yacsl::containers::StackArray<HANDLE, 2> output                = {};
  yacsl::containers::StackArray<HANDLE, 2> error                 = {};
#else
  alignas(64) yacsl::containers::StackArray<int, 3> original;
  yacsl::containers::StackArray<int, 2> input  = {};
  yacsl::containers::StackArray<int, 2> output = {};
  yacsl::containers::StackArray<int, 2> error  = {};
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
    yacsl::os::windows::try_win32(CreatePipe(&input[0], &input[1], nullptr, 0));
    yacsl::os::windows::try_win32(
        CreatePipe(&output[0], &output[1], nullptr, 0));
    yacsl::os::windows::try_win32(CreatePipe(&error[0], &error[1], nullptr, 0));
    yacsl::os::windows::try_win32(SetHandleInformation(
        input[0], HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
    yacsl::os::windows::try_win32(SetHandleInformation(
        output[1], HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));
    yacsl::os::windows::try_win32(SetHandleInformation(
        error[1], HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT));

    // NOLINTBEGIN(performance-no-int-to-ptr)
    yacsl::os::windows::try_win32(SetStdHandleEx(
        STD_INPUT_HANDLE, reinterpret_cast<HANDLE>(input[0]), &original[0]));
    yacsl::os::windows::try_win32(SetStdHandleEx(
        STD_OUTPUT_HANDLE, reinterpret_cast<HANDLE>(output[1]), &original[1]));
    yacsl::os::windows::try_win32(SetStdHandleEx(
        STD_ERROR_HANDLE, reinterpret_cast<HANDLE>(error[1]), &original[2]));

    //  NOLINTEND(performance-no-int-to-ptr)
#else
    // NOLINTNEXTLINE(android-cloexec-pipe)
    yacsl::os::unix::try_unix(::pipe(input.data()));
    // NOLINTNEXTLINE(android-cloexec-pipe)
    yacsl::os::unix::try_unix(::pipe(output.data()));
    // NOLINTNEXTLINE(android-cloexec-pipe)
    yacsl::os::unix::try_unix(::pipe(error.data()));

    yacsl::os::unix::try_unix(::fcntl(input[1], F_SETFD, FD_CLOEXEC));
    yacsl::os::unix::try_unix(::fcntl(output[0], F_SETFD, FD_CLOEXEC));
    yacsl::os::unix::try_unix(::fcntl(error[0], F_SETFD, FD_CLOEXEC));

    yacsl::os::unix::try_unix(::dup2(input[0], STDIN_FILENO));
    yacsl::os::unix::try_unix(::dup2(output[1], STDOUT_FILENO));
    yacsl::os::unix::try_unix(::dup2(error[1], STDERR_FILENO));

    yacsl::os::unix::try_unix(::close(input[0]));
    yacsl::os::unix::try_unix(::close(output[1]));
    yacsl::os::unix::try_unix(::close(error[1]));
#endif
  }

  void restore_original_io() {
#ifdef _WIN32
    yacsl::os::windows::try_win32(SetStdHandle(STD_INPUT_HANDLE, original[0]));
    yacsl::os::windows::try_win32(SetStdHandle(STD_OUTPUT_HANDLE, original[1]));
    yacsl::os::windows::try_win32(SetStdHandle(STD_ERROR_HANDLE, original[2]));

    yacsl::os::windows::try_win32(CloseHandle(input[0]));
    yacsl::os::windows::try_win32(CloseHandle(output[1]));
    yacsl::os::windows::try_win32(CloseHandle(error[1]));
#else
    yacsl::os::unix::try_unix(::dup2(original[0], STDIN_FILENO));
    yacsl::os::unix::try_unix(::dup2(original[1], STDOUT_FILENO));
    yacsl::os::unix::try_unix(::dup2(original[2], STDERR_FILENO));
    yacsl::os::unix::try_unix(::close(original[0]));
    yacsl::os::unix::try_unix(::close(original[1]));
    yacsl::os::unix::try_unix(::close(original[2]));
#endif
  }
};
} // namespace

template <typename T>
static T create_io(Type type, char const* path, auto& pipe) noexcept {
  if (type == Type::File) {
    return T(path, yacsl::os::File::OpenFlags::Create);
  }
  if (type == Type::OtherPipe) {
    return pipe;
  }
  return static_cast<Process::StdioType>(type);
}

static void
create_other_pipes(yacsl::containers::StackArray<Process, 3>& processes,
                   IoTypes                                    types) {
  if (types.input == Type::OtherPipe) {
    Process::SpawnOpts const stdio = {
        .input  = Process::Input::Inherit,
        .output = Process::Output::Pipe,
        .error  = Process::Output::Inherit,
    };
    processes[0] = Process::spawn({HELLO_WORLD_EXECUTABLE}, stdio).value();
  }
  Process::SpawnOpts const stdio = {
      .input  = Process::Input::Pipe,
      .output = Process::Output::Pipe,
      .error  = Process::Output::Inherit,
  };
  if (types.output == Type::OtherPipe) {
    processes[1] = Process::spawn({CAT_EXECUTABLE}, stdio).value();
  }
  if (types.error == Type::OtherPipe) {
    processes[2] = Process::spawn({CAT_EXECUTABLE}, stdio).value();
  }
}

static constexpr char const* CHILD_STDIN_FILE_PATH =
    YACSL_TESTS_DIR "/share/test_process/child_stdin.txt";
static constexpr char const* CHILD_STDOUT_FILE_PATH =
    YACSL_TESTS_DIR "/share/test_process/child_stdout.txt";
static constexpr char const* CHILD_STDERR_FILE_PATH =
    YACSL_TESTS_DIR "/share/test_process/child_stderr.txt";

static constexpr yacsl::ranges::StringView INPUT_DATA = "hello world\n";

static void write_input(Type type, Process* proc, Pipes* pipes) {
  if (type == Type::Inherit) {
#ifdef _WIN32
    DWORD written = 0;
    yacsl::os::windows::try_win32(WriteFile(pipes->input[1], INPUT_DATA.data(),
                                            INPUT_DATA.len(), &written,
                                            nullptr));
#else
    ::write(pipes->input[1], INPUT_DATA.data(), INPUT_DATA.len());
#endif
  } else if (type == Type::Pipe) {
    proc->get_stdin().write(INPUT_DATA.bytes());
  }
}

static yacsl::containers::String
get_pipe_output(Process::Output* output, yacsl::ranges::StringView expected) {
  yacsl::containers::String str;
  str.resize(expected.len() + 256);
  // NOLINTNEXTLINE(misc-const-correctness)
  yacsl::ranges::Slice<yacsl::u8> bytes      = str.bytes();
  yacsl::usize                    total_read = output->read(bytes);
  while (total_read < expected.len()) {
    total_read +=
        output->read({bytes.data() + total_read, bytes.len() - total_read});
  }
  str.resize(total_read);
  return str;
}

static void check_file(char const* path, yacsl::ranges::StringView expected) {
  int const fildes =
      yacsl::os::unix::try_unix(open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0644));

  yacsl::containers::String buf;

  ssize_t result = ::read(fildes, buf.grow(64), 64);

  while (result != 0) {
    yacsl::debug::assert_(result != -1);
#ifdef _WIN32
    result = ::read(fildes, buf.grow(static_cast<yacsl::usize>(result)),
                    static_cast<unsigned int>(result));
#else
    result = ::read(fildes, buf.grow(static_cast<yacsl::usize>(result)),
                    static_cast<size_t>(result));
#endif
  }
  yacsl::testing::expect_eq(result, 0);

  buf.resize(buf.len() - 64);
  yacsl::testing::expect_eq(buf.view(), expected);

  yacsl::os::unix::try_unix(close(fildes));
  yacsl::os::unix::try_unix(remove(path));
}

static void check_file_output(IoTypes types) noexcept {
  if (types.output == Type::File) {
    check_file(CHILD_STDOUT_FILE_PATH, "arguments (to stdout):\n"
                                       "arg1\n"
                                       "arg2\n"
                                       "argument with spaces\n"
                                       "exiting (to stdout)\n");
  }
  if (types.error == Type::File) {
    if (types.input == Type::Null) {
      check_file(CHILD_STDERR_FILE_PATH, "stdin (to stderr):\n"
                                         "exiting (to stderr)\n");
    } else {
      check_file(CHILD_STDERR_FILE_PATH, "stdin (to stderr):\n"
                                         "hello world\n"
                                         "exiting (to stderr)\n");
    }
  }
}

static void
check_other_pipe_output(yacsl::containers::StackArray<Process, 3>& processes,
                        IoTypes types) noexcept {
  if (types.output == Type::OtherPipe) {
    Process::ExitStatus const status = processes[1].wait();
    yacsl::testing::expect_eq(status.unread_stdout, "arguments (to stdout):\n"
                                                    "arg1\n"
                                                    "arg2\n"
                                                    "argument with spaces\n"
                                                    "exiting (to stdout)\n");
  }
  if (types.error == Type::OtherPipe) {
    Process::ExitStatus const status = processes[2].wait();
    if (types.input == Type::Null) {
      yacsl::testing::expect_eq(status.unread_stdout, "stdin (to stderr):\n"
                                                      "exiting (to stderr)\n");
    } else {
      yacsl::testing::expect_eq(status.unread_stdout, "stdin (to stderr):\n"
                                                      "hello world\n"
                                                      "exiting (to stderr)\n");
    }
  }
}

#ifdef _WIN32
static void check_fd_output(HANDLE file, yacsl::ranges::StringView expected) {
#else
static void check_fd_output(int file, yacsl::ranges::StringView expected) {
#endif
  yacsl::containers::String output;
#ifdef _WIN32
  DWORD   result  = 0;
  WINBOOL success = ReadFile(file, output.grow(512), 512, &result, nullptr);
  if (success == FALSE) [[unlikely]] {
    DWORD err = GetLastError();
    if (err != ERROR_BROKEN_PIPE) {
      yacsl::debug::panic("ReadFile() failed: {}", err);
    }
  }
#else
  ssize_t result = ::read(file, output.grow(512), 512);
#endif
#ifdef _WIN32
  while (result > 0) {
    success = ReadFile(file, output.grow(static_cast<yacsl::usize>(result)),
                       512, &result, nullptr);
    if (success == FALSE) [[unlikely]] {
      DWORD err = GetLastError();
      if (err != ERROR_BROKEN_PIPE) {
        yacsl::debug::panic("ReadFile() failed: {}", err);
      }
    }
  }
#else
  while (result > 0) {
    result = ::read(file, output.grow(static_cast<yacsl::usize>(result)), 512);
  }
  yacsl::debug::assert_(result != -1);
#endif
  output.resize(output.len() - 512);
  yacsl::testing::expect_eq(output.view(), expected);
}

static void check_inherited_output(IoTypes types, Pipes const& pipes) {
  if (types.output == Type::Inherit) {
    check_fd_output(pipes.output[0], "arguments (to stdout):\n"
                                     "arg1\n"
                                     "arg2\n"
                                     "argument with spaces\n"
                                     "exiting (to stdout)\n");
  }
  if (types.error == Type::Inherit) {
    if (types.input == Type::Null) {
      check_fd_output(pipes.error[0], "stdin (to stderr):\n"
                                      "exiting (to stderr)\n");
    } else {
      check_fd_output(pipes.error[0], "stdin (to stderr):\n"
                                      "hello world\n"
                                      "exiting (to stderr)\n");
    }
  }
}

namespace {
struct RuntimeOutput {
  yacsl::ranges::StringView out;
  yacsl::ranges::StringView err;
};
} // namespace

static void check_piped_output(IoTypes types, RuntimeOutput runtime_out,
                               Process::ExitStatus const& status) {
  if (types.output == Type::Pipe) {
    if (types.input == Type::Pipe || types.input == Type::Inherit) {
      yacsl::testing::expect_eq(runtime_out.out, "arguments (to stdout):\n"
                                                 "arg1\n"
                                                 "arg2\n"
                                                 "argument with spaces\n");
      yacsl::testing::expect_eq(status.unread_stdout, "exiting (to stdout)\n");
    } else {
      yacsl::testing::expect_eq(runtime_out.out, "arguments (to stdout):\n"
                                                 "arg1\n"
                                                 "arg2\n"
                                                 "argument with spaces\n"
                                                 "exiting (to stdout)\n");
      yacsl::testing::expect_eq(status.unread_stdout, "");
    }
  } else {
    yacsl::testing::expect_eq(status.unread_stdout, "");
  }
  if (types.error == Type::Pipe) {
    if (types.input == Type::Pipe || types.input == Type::Inherit) {
      yacsl::testing::expect_eq(runtime_out.err, "stdin (to stderr):\n"
                                                 "hello world\n");
      yacsl::testing::expect_eq(status.unread_stderr, "exiting (to stderr)\n");
    } else if (types.input == Type::Null) {
      yacsl::testing::expect_eq(runtime_out.err, "stdin (to stderr):\n"
                                                 "exiting (to stderr)\n");
      yacsl::testing::expect_eq(status.unread_stderr, "");
    } else {
      yacsl::testing::expect_eq(runtime_out.err, "stdin (to stderr):\n"
                                                 "hello world\n"
                                                 "exiting (to stderr)\n");
      yacsl::testing::expect_eq(status.unread_stderr, "");
    }
  } else {
    yacsl::testing::expect_eq(status.unread_stderr, "");
  }
}

static void test_run_c(IoTypes types) {
  Pipes pipes;

  yacsl::containers::StackArray<Process, 3> other_pipes;
  create_other_pipes(other_pipes, types);

  Process::SpawnOpts const stdio = {
      .input = create_io<Process::Input::Init>(
          types.input, CHILD_STDIN_FILE_PATH, other_pipes[0].get_stdout()),
      .output = create_io<Process::Output::Init>(
          types.output, CHILD_STDOUT_FILE_PATH, other_pipes[1].get_stdin()),
      .error = create_io<Process::Output::Init>(
          types.error, CHILD_STDERR_FILE_PATH, other_pipes[2].get_stdin()),
  };

  auto process = Process::spawn(
                     {
                         STD_STREAMS_EXECUTABLE,
                         "arg1",
                         "arg2",
                         "argument with spaces",
                     },
                     stdio)
                     .value();

  write_input(types.input, &process, &pipes);

  yacsl::containers::String piped_output;
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

  yacsl::containers::String piped_error;
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
  yacsl::os::windows::try_win32(CloseHandle(pipes.input[1]));
#else
  yacsl::os::unix::try_unix(::close(pipes.input[1]));
#endif
  Process::ExitStatus const status = process.wait();
  pipes.restore_original_io();

  check_file_output(types);
  check_other_pipe_output(other_pipes, types);
  check_inherited_output(types, pipes);
  check_piped_output(
      types, {.out = piped_output.view(), .err = piped_error.view()}, status);

#ifdef _WIN32
  yacsl::os::windows::try_win32(CloseHandle(pipes.output[0]));
  yacsl::os::windows::try_win32(CloseHandle(pipes.error[0]));
#else
  yacsl::os::unix::try_unix(::close(pipes.output[0]));
  yacsl::os::unix::try_unix(::close(pipes.error[0]));
#endif
}

static IoTypes& io_types() {
  static IoTypes instance = {};
  return instance;
}

static void create_input_file() noexcept {
  int const file = ::open(CHILD_STDIN_FILE_PATH,
                          O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC, 0644);
  yacsl::debug::assert_(file != -1);
  [[maybe_unused]] ssize_t const written =
      ::write(file, INPUT_DATA.data(), INPUT_DATA.len());
  yacsl::debug::assert_eq(static_cast<yacsl::usize>(written), INPUT_DATA.len());
  yacsl::debug::assert_(close(file) == 0);
}

static void test_cwd(yacsl::testing::Test& /*test*/) noexcept {
#ifdef _WIN32
  auto process = Process::spawn({PWD_EXECUTABLE},
                                {
                                    .output = Process::Output::Pipe,
                                    .error  = Process::Output::Inherit,
                                    .cwd    = "C:\\",
                                })
                     .value();
  yacsl::testing::expect_eq(process.wait().unread_stdout, "cwd: 'C:\\'\r\n");
#else
  auto process = Process::spawn({PWD_EXECUTABLE},
                                {
                                    .output = Process::Output::Pipe,
                                    .error  = Process::Output::Inherit,
                                    .cwd    = "/",
                                })
                     .value();
  yacsl::testing::expect_eq(process.wait().unread_stdout, "cwd: '/'\n");
#endif
}

int main() {
  yacsl::containers::StackArray tests = {
      yacsl::testing::Test("current working directory", test_cwd),
  };
  for (auto& test : tests) {
    test.run();
  }

  create_input_file();

  (void)yacsl::debug::Logger::get_default_instance();

  yacsl::containers::StackArray<Type, 5> const types = {
      Type::Null, Type::Inherit, Type::Pipe, Type::File, Type::OtherPipe,
  };
  for (Type stdin_type : types) {
    for (Type stdout_type : types) {
      for (Type stderr_type : types) {
        io_types() = {
            .input  = stdin_type,
            .output = stdout_type,
            .error  = stderr_type,
        };

        yacsl::testing::Test test(yacsl::fmt::format("{}, {}, {}", stdin_type,
                                                     stdout_type, stderr_type)
                                      .c_str(),
                                  [](yacsl::testing::Test& /*test*/) {
                                    yacsl::debug::flush_logger();
                                    test_run_c(io_types());
                                  });

        test.run();
      }
    }
  }

  yacsl::testing::expect(remove(CHILD_STDIN_FILE_PATH) == 0);
}

#else

#include "yacsl/debug/log.hpp"

int main() {
  yacsl::debug::log_info(
      "test_process does nothing when compiling with emscripten.");
}

#endif
