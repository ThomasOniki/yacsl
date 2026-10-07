#pragma once

#include "yacsl/ranges/slice.hpp"
#include "yacsl/testing/test.hpp"
#include "yacsl/type_aliases.hpp"
#include <stdarg.h>

constexpr int
run_tests(yacsl::ranges::Slice<yacsl::testing::Test, yacsl::u32> tests) {
  int ret = 0;
  for (auto& test : tests) {
    if (test.run_quietly() == yacsl::testing::Test::Result::Failure) {
      ret = 1;
    }
  }
  return ret;
}

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
