#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  char buf[1024];
  if (getcwd(buf, sizeof(buf)) == NULL) {
    perror("getcwd()");
    return 1;
  }
  printf("cwd: '%s'\n", buf);
}
