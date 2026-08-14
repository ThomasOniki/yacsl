#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
  enum {
    BUFFER_SIZE = 4096,
  };
  char    buf[BUFFER_SIZE];
  ssize_t result = read(STDIN_FILENO, buf, BUFFER_SIZE);
  while (result > 0) {
    write(STDOUT_FILENO, buf, (size_t)result);
    result = read(STDIN_FILENO, buf, BUFFER_SIZE);
  }
  assert(result != -1);
}

// int main(void) {}
