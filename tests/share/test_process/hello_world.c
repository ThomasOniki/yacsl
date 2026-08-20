#include <stdio.h>

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

  puts("hello world");
}
