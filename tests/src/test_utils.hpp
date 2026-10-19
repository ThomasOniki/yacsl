#pragma once

#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/testing/run.hpp"
#include <stdarg.h>

#define STRINGIFY_IMPL(...) #__VA_ARGS__
#define STRINGIFY(...) STRINGIFY_IMPL(__VA_ARGS__)

#define EXPAND(...) __VA_ARGS__

#define FAIL(...)                                                              \
  do {                                                                         \
    yacsl::testing::fail_quietly();                                            \
    fprintf(stderr, "Failed test at " __FILE__                                 \
                    ":" STRINGIFY(__LINE__) "\n" __VA_ARGS__);                 \
  } while (false)

#define EXPECT(cond, ...)                                                      \
  do {                                                                         \
    bool const _result = cond;                                                 \
    if (!_result) [[unlikely]] {                                               \
      yacsl::testing::fail_quietly();                                          \
      fprintf(stderr, "EXPECT(" #cond ") failed at " __FILE__                  \
                      ":" STRINGIFY(__LINE__) "\n" __VA_ARGS__);               \
    }                                                                          \
  } while (false)

template <typename F, typename... Args>
void run_test(char const* name, F&& func, Args&&... args) noexcept {
  printf("Running test '%s'...\n", name);
  yacsl::testing::run_quitely(yacsl::memory::forward<F>(func),
                              yacsl::memory::forward<Args>(args)...);
  if (yacsl::testing::status()) {
    printf("Test '%s' succeeded!\n", name);
  } else {
    fprintf(stderr, "Test '%s' failed!\n", name);
  }
}
