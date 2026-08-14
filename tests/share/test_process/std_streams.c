#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, const char* const* argv) {
  assert(fputs("arguments (to stdout):\n", stdout) >= 0);
  for (int i = 1; i < argc; ++i) {
    assert(puts(argv[i]) >= 0);
  }
  assert(fflush(stdout) == 0);

  assert(fputs("stdin (to stderr):\n", stderr) >= 0);
  enum {
    BUFFER_SIZE = 4096,
  };
  char    buf[BUFFER_SIZE];
  ssize_t result = read(STDIN_FILENO, buf, BUFFER_SIZE);
  while (result > 0) {
    assert(fwrite(buf, (size_t)result, 1, stderr) == 1);
    result = read(STDIN_FILENO, buf, BUFFER_SIZE);
  }
  assert(result != -1);
  assert(fputs("exiting (to stdout)\n", stdout) >= 0);
  assert(fputs("exiting (to stderr)\n", stderr) >= 0);
}
