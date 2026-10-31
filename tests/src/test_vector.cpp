#include "yacsl/testing/run.hpp"
#include <stdio.h>
#include <stdlib.h>

int main() {
  fputs("TODO: proper vector tests\n", stderr);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
