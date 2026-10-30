#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/testing/run.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define FAIL(...)                                                              \
  do {                                                                         \
    yacsl::testing::fail_quietly();                                            \
    fprintf(stderr, "Failed test at %s:%d\n", __FILE__, __LINE__);             \
    fprintf(stderr, "" __VA_ARGS__);                                           \
  } while (false)

#define EXPECT(cond, ...)                                                      \
  do {                                                                         \
    bool const _result = cond;                                                 \
    if (!_result) [[unlikely]] {                                               \
      yacsl::testing::fail_quietly();                                          \
      fprintf(stderr, "EXPECT(%s) failed at %s:%d\n", #cond, __FILE__,         \
              __LINE__);                                                       \
      fprintf(stderr, "" __VA_ARGS__);                                         \
    }                                                                          \
  } while (false)

#define EXPECT_STREQ(lhs, rhs, ...)                                            \
  do {                                                                         \
    if (strcmp(lhs, rhs) != 0) [[unlikely]] {                                  \
      yacsl::testing::fail_quietly();                                          \
      fprintf(stderr,                                                          \
              "EXPECT_STREQ(%s, %s) failed at %s:%d\nlhs: '%s'\nrhs: '%s'\n",  \
              #lhs, #rhs, __FILE__, __LINE__, lhs, rhs);                       \
      fprintf(stderr, "" __VA_ARGS__);                                         \
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
  constexpr DynamicArray() noexcept = default;
  constexpr ~DynamicArray() noexcept { delete[] ptr; }

  DynamicArray(DynamicArray&&)                 = delete;
  DynamicArray(DynamicArray const&)            = delete;
  DynamicArray& operator=(DynamicArray&&)      = delete;
  DynamicArray& operator=(DynamicArray const&) = delete;

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

  constexpr DynamicCString() noexcept = default;
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

  char*  ptr = nullptr;
  size_t len = 0;
};

class SMFTracker {
public:
  explicit SMFTracker(char const* name) : name_(name) {
    HISTORY.printf_to("constructed '%s'\n", name);
  }
  SMFTracker(SMFTracker const& other) {
    name_.printf_to("%s (copy)", other.name_.ptr);
    HISTORY.printf_to("copy constructed '%s'\n", other.name_.ptr);
  }
  SMFTracker(SMFTracker&& other) noexcept {
    name_.printf_to("%s (moved)", other.name_.ptr);
    HISTORY.printf_to("move constructed '%s'\n", other.name_.ptr);
  }
  ~SMFTracker() noexcept { HISTORY.printf_to("destroyed '%s'\n", name_.ptr); }
  SMFTracker& operator=(this SMFTracker& self, SMFTracker const& other) {
    if (&other == &self) {
      return self;
    }
    self.name_.clear();
    self.name_.printf_to("%s (copy assigned)", other.name_.ptr);
    HISTORY.printf_to("copy assigned '%s'\n", other.name_.ptr);
    return self;
  }
  [[maybe_unused]] SMFTracker& operator=(this SMFTracker& self,
                                         SMFTracker&&     other) noexcept {
    self.name_.clear();
    self.name_.printf_to("%s (move assigned)", other.name_.ptr);
    HISTORY.printf_to("move assigned '%s'\n", other.name_.ptr);
    return self;
  }

public:
  inline static DynamicCString HISTORY;
  DynamicCString               name_;
};
