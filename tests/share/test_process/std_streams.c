#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#define CHECK(x)                                                                                   \
  do {                                                                                             \
    int _result = x;                                                                               \
    if (_result < 0) {                                                                             \
      perror(#x);                                                                                  \
      abort();                                                                                     \
    }                                                                                              \
  } while (0)

int main(int argc, const char* const* argv) {
#ifdef _WIN32
  // no "\n" to "\r\n" conversions
  _setmode(STDOUT_FILENO, _O_BINARY);
  _setmode(STDERR_FILENO, _O_BINARY);
#endif

  CHECK(fputs("arguments (to stdout):\n", stdout));
  for (int i = 1; i < argc; ++i) {
    CHECK(puts(argv[i]));
  }
  CHECK(fflush(stdout));

  CHECK(fputs("stdin (to stderr):\n", stderr));
  enum {
    BUFFER_SIZE = 4096,
  };
  char    buf[BUFFER_SIZE];
  ssize_t result = read(STDIN_FILENO, buf, BUFFER_SIZE);
  while (result > 0) {
    CHECK(fwrite(buf, (size_t)result, 1, stderr));
    result = read(STDIN_FILENO, buf, BUFFER_SIZE);
  }
  assert(result != -1);
  CHECK(fputs("exiting (to stdout)\n", stdout));
  CHECK(fputs("exiting (to stderr)\n", stderr));
}
