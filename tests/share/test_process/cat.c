#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

int main(void) {
#ifdef _WIN32
  // no "\n" to "\r\n" conversions
  _setmode(STDOUT_FILENO, _O_BINARY);
  _setmode(STDERR_FILENO, _O_BINARY);
#endif

  enum {
    BUFFER_SIZE = 4096,
  };
  char    buf[BUFFER_SIZE];
  ssize_t result = read(STDIN_FILENO, buf, BUFFER_SIZE);
  while (result > 0) {
    write(STDOUT_FILENO, buf, (size_t)result);
    result = read(STDIN_FILENO, buf, BUFFER_SIZE);
  }
  if (result < 0) {
    perror("read");
    return -1;
  }
}

// int main(void) {}
