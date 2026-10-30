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
    if (((lhs) == nullptr && (rhs) != nullptr && (rhs)[0] != '\0') ||          \
        ((rhs) == nullptr && (lhs) != nullptr && (lhs)[0] != '\0') ||          \
        ((lhs) != nullptr && (rhs) != nullptr) && strcmp(lhs, rhs) != 0)       \
        [[unlikely]] {                                                         \
      yacsl::testing::fail_quietly();                                          \
      fprintf(stderr,                                                          \
              "EXPECT_STREQ(%s, %s) failed at %s:%d\nlhs: '%s'\nrhs: '%s'\n",  \
              #lhs, #rhs, __FILE__, __LINE__,                                  \
              (lhs) == nullptr ? "(nullptr)" : (lhs),                          \
              (rhs) == nullptr ? "(nullptr)" : (rhs));                         \
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

struct TestAllocator {
  using SizeT = size_t;

  [[nodiscard]] constexpr bool operator==(this TestAllocator const& self,
                                          TestAllocator const&      other) {
    return &self == &other;
  }

  yacsl::ranges::Slice<yacsl::u8, SizeT>
  allocate(this TestAllocator& /*self*/, SizeT size,
           yacsl::memory::Alignment align) noexcept {
#ifdef _WIN32
    void* ret = _aligned_malloc(static_cast<size_t>(align), size);
#else
    void* ret = aligned_alloc(static_cast<size_t>(align), size);
#endif
    dynarray_remove(freed, ret);
    dynarray_insert(allocated, ret);
    HISTORY.printf_to("allocated %zu bytes\n", size);
    POINTER_HIST.push(ret);
    return {
        static_cast<yacsl::u8*>(ret),
        size,
    };
  }

  void deallocate(this TestAllocator& /*self*/,
                  yacsl::ranges::Slice<yacsl::u8, SizeT> mem) noexcept {
    if (mem.data() != nullptr) {
      EXPECT(!dynarray_has(freed, mem.data()), "double free\n");
      EXPECT(dynarray_has(allocated, mem.data()),
             "Tried to free unallocated pointer.\n");
      dynarray_insert(freed, mem.data());

#ifdef _WIN32
      _aligned_free(mem.data());
#else
      free(mem.data());
#endif
    }
    HISTORY.printf_to("deallocated %zu bytes\n", mem.len());
    POINTER_HIST.push(mem.data());
  }

  yacsl::ranges::Slice<yacsl::u8, SizeT>
  resize_in_place(this TestAllocator& /*self*/,
                  yacsl::ranges::Slice<yacsl::u8, SizeT> /*mem*/,
                  SizeT /*new_size*/) {
    return {nullptr, 0UZ};
  }

  static constexpr bool dynarray_has(DynamicArray<void*> const& arr,
                                     void*                      ptr) {
    for (size_t i = 0; i < arr.len; ++i) {
      if (arr.ptr[i] == ptr) {
        return true;
      }
    }
    return false;
  }
  static constexpr void dynarray_insert(DynamicArray<void*>& arr,
                                        void*                to_insert) {
    for (size_t i = 0; i < arr.len; ++i) {
      if (arr.ptr[i] == nullptr) {
        arr.ptr[i] = to_insert;
        return;
      }
    }
    arr.push(to_insert);
  }
  static constexpr bool dynarray_remove(DynamicArray<void*>& arr,
                                        void*                to_remove) {
    for (size_t i = 0; i < arr.len; ++i) {
      if (arr.ptr[i] == to_remove) {
        arr.ptr[i] = nullptr;
        return true;
      }
    }
    return false;
  }

  inline static DynamicArray<void*> freed;
  inline static DynamicArray<void*> allocated;

  static constexpr void clear_history() {
    HISTORY.clear();
    POINTER_HIST.clear();
  }

  inline static DynamicArray<void*> POINTER_HIST;
  inline static DynamicCString      HISTORY;

  SMFTracker smf_tracker_ = SMFTracker("allocator");
};

using TestAllocatorSingleton =
    yacsl::memory::SingletonAllocator<TestAllocator, "main", true>;
