#ifndef __EMSCRIPTEN__

import base.containers.stack_array;
import base.ranges.string_view;
import base.os.common;
import base.os.file;
import base.os.process;
import base.memory.lifetimes;
#include "test_utils.hpp"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef S_IRUSR
static constexpr int OPEN_PERMISSIONS = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
#else
static constexpr int OPEN_PERMISSIONS = 0;
#endif

#define TRY_SYSCALL(syscall)                                                                       \
  do {                                                                                             \
    if ((syscall) == -1) [[unlikely]] {                                                            \
      perror(#syscall);                                                                            \
      abort();                                                                                     \
    }                                                                                              \
  } while (false)

static int redirect(int stream, const char* path) {
  // NOLINTNEXTLINE(android-cloexec-dup)
  const int old = dup(stream);
  assert(old != -1);
  // NOLINTNEXTLINE(android-cloexec-open)
  const int file = open(path, O_WRONLY | O_CREAT | O_TRUNC, OPEN_PERMISSIONS);
  assert(file != -1);
  TRY_SYSCALL(dup2(file, stream));
  TRY_SYSCALL(close(file));
  return old;
}

static constexpr const char* STDOUT_FILE_PATH = "test_process_stdout.txt";
static constexpr const char* STDERR_FILE_PATH = "test_process_stderr.txt";

namespace {

struct Redirected {
  Redirected() noexcept
      : original_stdout(redirect(STDOUT_FILENO, STDOUT_FILE_PATH)),
        original_stderr(redirect(STDERR_FILENO, STDERR_FILE_PATH)) {}

  ~Redirected() noexcept {
    TRY_SYSCALL(dup2(original_stdout, STDOUT_FILENO));
    TRY_SYSCALL(close(original_stdout));
    (void)fputs("stdout restored\n", stdout);

    TRY_SYSCALL(dup2(original_stderr, STDERR_FILENO));
    TRY_SYSCALL(close(original_stderr));
    (void)fputs("stderr restored\n", stderr);
  }

  Redirected(const Redirected&)            = delete;
  Redirected(Redirected&&)                 = delete;
  Redirected& operator=(const Redirected&) = delete;
  Redirected& operator=(Redirected&&)      = delete;

  [[nodiscard]] static Redirected& get() {
    static Redirected instance = Redirected();
    return instance;
  }

  void restore_original_stderr() {
    // NOLINTNEXTLINE(android-cloexec-dup)
    stderr_file = dup(STDERR_FILENO);
    assert(stderr_file != -1);
    TRY_SYSCALL(dup2(original_stderr, STDERR_FILENO));
    TRY_SYSCALL(close(original_stderr));
    original_stderr = -1;
  }
  void restore_file_stderr() {
    // NOLINTNEXTLINE(android-cloexec-dup)
    original_stderr = dup(STDERR_FILENO);
    assert(original_stderr != -1);
    TRY_SYSCALL(dup2(stderr_file, STDERR_FILENO));
    TRY_SYSCALL(close(stderr_file));
    stderr_file = -1;
  }
  static void maybe_rewind_stdout_stderr(const char* path) {
    if (path != nullptr) {
      if (strcmp(path, STDOUT_FILE_PATH) == 0) {
        TRY_SYSCALL(lseek(STDOUT_FILENO, 0, SEEK_SET));
      } else if (strcmp(path, STDERR_FILE_PATH) == 0) {
        TRY_SYSCALL(lseek(STDERR_FILENO, 0, SEEK_SET));
      }
    }
  }

  alignas(16) int original_stdout;
  int original_stderr;
  int stderr_file = -1;
};

} // namespace

static int check_file(const char* path, base::StringView expected) {
  int ret = 0;

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
  const int fildes =
      path == nullptr ? STDIN_FILENO : open(path, O_RDWR | O_CREAT | O_CLOEXEC, OPEN_PERMISSIONS);
  assert(fildes != -1);

  base::StackArray<char, 65, size_t> buf            = {};
  base::StackArray<char, 65, size_t> expected_chunk = {};

  size_t read_sz = 1;
  size_t size    = 0;

  while (read_sz > 0) {
    read_sz = static_cast<size_t>(read(fildes, buf.data(), 64));
    assert(read_sz <= 64); // If greater, it overflowed from -1.
    buf[read_sz] = '\0';

    const size_t last_sz = size;
    size += read_sz;
    if (size <= expected.size()) {
      memcpy(expected_chunk.data(), expected.data() + last_sz, read_sz);
      expected_chunk[read_sz] = '\0';
    } else if (size - read_sz < expected.size()) {
      memcpy(expected_chunk.data(), expected.data() + last_sz, expected.size() - last_sz);
    } else {
      expected_chunk[0] = '\0';
    }

    EXPECT_STREQ(buf.data(), expected_chunk.data());
  }
  EXPECT_EQ(read_sz, 0);

  EXPECT_EQ(size, expected.size());

  if (path != nullptr) {
    TRY_SYSCALL(ftruncate(fildes, 0));
    TRY_SYSCALL(close(fildes));
  }

  return ret;
}

static constexpr base::FileOpenFlags FLAGS =
    base::FileOpenFlags::Write | base::FileOpenFlags::Create | base::FileOpenFlags::Trunc;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_FILE(path, contents)                                                                 \
  do {                                                                                             \
    Redirected::get().restore_original_stderr();                                                   \
    EXPECT(check_file(path, contents) == 0);                                                       \
    Redirected::get().restore_file_stderr();                                                       \
    Redirected::get().maybe_rewind_stdout_stderr(path);                                            \
  } while (false)

using base::Process;
using Stdio = Process::Stdio;
using enum Stdio::Type;

static int test_redirect_stdout_and_stderr() {
  int ret = 0;

  // Redirects
  (void)Redirected::get();

  (void)fputs("stdout print test1\n", stdout);
  (void)fflush(stdout);
  CHECK_FILE(STDOUT_FILE_PATH, "stdout print test1\n");
  (void)fputs("stdout print test2\n", stdout);
  (void)fflush(stdout);
  CHECK_FILE(STDOUT_FILE_PATH, "stdout print test2\n");

  (void)fputs("stderr print test1\n", stderr);
  CHECK_FILE(STDERR_FILE_PATH, "stderr print test1\n");
  (void)fputs("stderr print test2\n", stderr);
  CHECK_FILE(STDERR_FILE_PATH, "stderr print test2\n");

  return ret;
}

static int test_echo() {
  int ret = 0;

  Process echo;
  echo.start({"echo", "hello", "world"}, ParentStdin, ParentStdout, ParentStderr);
  echo.wait();
  CHECK_FILE(STDOUT_FILE_PATH, "hello world\n");

  return ret;
}

static int test_compile_c() {
  int ret = 0;

  static constexpr base::StringView C_CODE = R"(#include <stdio.h>

int main() {
	fputs("stdout\n", stdout);
	fflush(stdout);
	fputs("stderr\n", stderr);
})";

  Process create;
  create.start({"echo", "-n", C_CODE}, ParentStdin, Stdio("test_process_hello.c", FLAGS),
               ParentStderr);
  create.wait();

  Process compile;
  compile.start({"gcc", "test_process_hello.c", "-o", "test_process_hello"}, ParentStdin,
                ParentStdout, ParentStderr);
  compile.wait();

  // check_file() now so it deletes it after compiling
  CHECK_FILE("test_process_hello.c", C_CODE);

  return ret;
}

static int test_run_c(Stdio&& stdout_opts, Stdio&& stderr_opts, const char* out_file,
                      const char* err_file) {
  int     ret = 0;
  Process process;
  process.start({"./test_process_hello"}, ParentStdin, base::move(stdout_opts),
                base::move(stderr_opts));

  if (out_file == nullptr && err_file == nullptr) {
    CHECK_FILE(out_file, "stdout\nstderr\n");
  } else {
    if (out_file == nullptr) {
      CHECK_FILE(out_file, "stdout\n");
    }
    if (err_file == nullptr) {
      CHECK_FILE(err_file, "stderr\n");
    }
  }

  process.wait();

  if (out_file != nullptr && err_file != nullptr && strcmp(out_file, err_file) == 0) {
    CHECK_FILE(out_file, "stdout\nstderr\n");
  } else {
    if (out_file != nullptr) {
      CHECK_FILE(out_file, "stdout\n");
    }
    if (err_file != nullptr) {
      CHECK_FILE(err_file, "stderr\n");
    }
  }

  return ret;
}

static int test_all_run_c() {
  int ret = 0;
  EXPECT(test_run_c(ParentStdout, ParentStderr, STDOUT_FILE_PATH, STDERR_FILE_PATH) == 0);
  EXPECT(test_run_c(ParentStderr, ParentStdout, STDERR_FILE_PATH, STDOUT_FILE_PATH) == 0);
  EXPECT(test_run_c(ParentStdout, ParentStdout, STDOUT_FILE_PATH, STDOUT_FILE_PATH) == 0);
  EXPECT(test_run_c(ParentStderr, ParentStderr, STDERR_FILE_PATH, STDERR_FILE_PATH) == 0);
  EXPECT(test_run_c(ParentStderr, Stdio("test_process_err", FLAGS), STDERR_FILE_PATH,
                    "test_process_err") == 0);
  EXPECT(test_run_c(ParentStderr, ParentStdin, STDERR_FILE_PATH, nullptr) == 0);
  EXPECT(test_run_c(ParentStdin, ParentStdin, nullptr, nullptr) == 0);
  EXPECT(test_run_c(ParentStdin, ParentStdout, nullptr, STDOUT_FILE_PATH) == 0);
  return ret;
}

static int test_process_pipe() {
  int     ret = 0;
  Process echo;
  echo.start({"echo", "hello", "world"}, ParentStdin, Buffer, ParentStderr);
  Process cat;
  cat.start({"cat"}, Stdio(echo.get_stdout()), Buffer, ParentStderr);
  Process tee;
  tee.start({"tee", "test_process_echoed.txt"}, Stdio(cat.get_stdout()), ParentStdout,
            ParentStderr);
  tee.wait();
  CHECK_FILE("test_process_echoed.txt", "hello world\n");
  CHECK_FILE(STDOUT_FILE_PATH, "hello world\n");
  return ret;
}

static int test_buffer() {
  int                        ret = 0;
  base::StackArray<char, 64> buf{};

  using base::litterals::operator""_sv;

  Process cat1;
  cat1.start({"cat"}, Buffer, Buffer, ParentStderr);
  cat1.get_stdin().write("buffer test"_sv.bytes());
  EXPECT_EQ(cat1.get_stdout().read({buf.bytes().data(), buf.bytes().size()}), 11);
  EXPECT_STREQ(buf.data(), "buffer test");
  cat1.wait();
  return ret;
}

int main() {
  int ret = 0;

  {
    EXPECT(test_redirect_stdout_and_stderr() == 0);
    EXPECT(test_echo() == 0);
    EXPECT(test_compile_c() == 0);
    EXPECT(test_all_run_c() == 0);
    EXPECT(test_process_pipe() == 0);
    EXPECT(test_buffer() == 0);
  }

  return ret;
}

#else

import base.logging.logger;

int main() { base::log_info("test_process does nothing when compiling with emscripten."); }

#endif
