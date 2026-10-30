#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/testing/run.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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

template <typename T> struct DynamicArray {
  DynamicArray(DynamicArray&&)                 = delete;
  DynamicArray(DynamicArray const&)            = delete;
  DynamicArray& operator=(DynamicArray&&)      = delete;
  DynamicArray& operator=(DynamicArray const&) = delete;
  constexpr ~DynamicArray() noexcept { delete[] ptr; }

  constexpr void push(this DynamicArray& self, T value) {
    T* mem = new T[self.len + 1];
    for (size_t i = 0; i < self.len; ++i) {
      mem[i] = self.ptr[i];
    }
    delete[] self.ptr;
    self.ptr             = mem;
    self.ptr[self.len++] = value;
  }

  constexpr void clear(this DynamicArray& self) {
    delete[] self.ptr;
    self.ptr = nullptr;
    self.len = 0;
  }

  T*     ptr = nullptr;
  size_t len = 0;
};

struct DynamicCString {
  constexpr DynamicCString(char const* cstr) : len(strlen(cstr)) {
    ptr = new char[len + 1];
    memcpy(ptr, cstr, len + 1);
  }

  constexpr ~DynamicCString() noexcept { delete[] ptr; }

  DynamicCString(DynamicCString&&)                 = delete;
  DynamicCString(DynamicCString const&)            = delete;
  DynamicCString& operator=(DynamicCString&&)      = delete;
  DynamicCString& operator=(DynamicCString const&) = delete;

  constexpr void clear(this DynamicCString& self) noexcept {
    delete[] self.ptr;
    self.ptr = nullptr;
    self.len = 0;
  }

  template <typename... Args>
  YACSL_GCC_OR_CLANG_ATTR([[gnu::format(printf, 2, 3)]])
  // NOLINTNEXTLINE(modernize-avoid-variadic-functions)
  void printf_to(this DynamicCString& self, char const* fmt, ...) {
    char*   formatted = nullptr;
    va_list vargs     = {};
    va_start(vargs, fmt);
    auto const len = static_cast<size_t>(vasprintf(&formatted, fmt, vargs));
    va_end(vargs);

    char* mem = new char[self.len + len + 1];
    memcpy(mem, self.ptr, self.len);
    memcpy(mem + self.len, formatted, len);

    free(formatted);
    delete[] self.ptr;

    self.ptr = mem;
    self.len += len;
    self.ptr[self.len] = '\0';
  }

  constexpr bool operator==(this DynamicCString const& self,
                            DynamicCString const&      other) {
    return self.len == other.len && memcmp(self.ptr, other.ptr, self.len) == 0;
  }

  char*  ptr;
  size_t len;
};
