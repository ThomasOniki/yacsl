#include "yacsl/containers/stack_array.hpp"
#include "yacsl/memory/arena.hpp"
#include "yacsl/testing/test.hpp"
#include <cstdint>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

constexpr uint32_t iteration_count = 10000;

namespace {
struct CtorCounter {
  inline static uint32_t ctor_call_count = 0;
  explicit CtorCounter(uint32_t num) : i_(num) { ++ctor_call_count; }
  [[maybe_unused]] CtorCounter() { ++ctor_call_count; }
  uint32_t i_ = 0;
};

struct Aligned {
  alignas(64) char data;
};
} // namespace

static void test_ctors(yacsl::testing::Test& test) {
  CtorCounter::ctor_call_count = 0;

  CtorCounter* ptrs[iteration_count] = {};
  for (uint32_t i = 0; i < iteration_count; ++i) {
    ptrs[i] = &yacsl::memory::MainArenaAllocator::get().push<CtorCounter>(i);
  }

  if (CtorCounter::ctor_call_count != iteration_count) {
    test.fail_quietly();
    fprintf(stderr, "%u constructors were called instead of %u\n",
            CtorCounter::ctor_call_count, iteration_count);
  }

  for (uint32_t i = 0; i < iteration_count; ++i) {
    if (ptrs[i]->i_ != i) {
      test.fail_quietly();
      fprintf(stderr,
              "%uth arena allocated object held value %u instead of %u\n", i,
              ptrs[i]->i_, i);
    }
  }

  CtorCounter::ctor_call_count = 0;

  for (auto& ptr : ptrs) {
    ptr = yacsl::memory::MainArenaAllocator::get()
              .push_array<CtorCounter>(10)
              .data();
  }

  if (CtorCounter::ctor_call_count != iteration_count * 10) {
    test.fail_quietly();
    fprintf(stderr, "%u constructors were called instead of %u\n",
            CtorCounter::ctor_call_count, iteration_count * 10);
  }

  for (uint32_t i = 0; i < iteration_count; ++i) {
    for (uint32_t j = 0; j < 10; ++j) {
      if (ptrs[j]->i_ != 0) {
        test.fail_quietly();
        fprintf(stderr,
                "%uth arena allocated object held value %u instead of 0\n",
                i * 10 + j, ptrs[j]->i_);
      }
    }
  }
}

static bool is_aligned(Aligned const* ptr) {
  auto const uptr = reinterpret_cast<uintptr_t>(ptr);
  return uptr % alignof(Aligned) == 0;
}

static void test_align(yacsl::testing::Test& test) {
  (void)yacsl::memory::MainArenaAllocator::get().push_array<char>(15);

  Aligned const* aligned =
      &yacsl::memory::MainArenaAllocator::get().push<Aligned>();

  if (!is_aligned(aligned)) {
    test.fail_quietly();
    fprintf(stderr,
            "Pointer %p allocated with Arena::push is not aligned properly.\n",
            static_cast<void const*>(aligned));
  }

  (void)yacsl::memory::MainArenaAllocator::get().push_array<char>(15);

  aligned =
      yacsl::memory::MainArenaAllocator::get().push_array<Aligned>(10).data();

  fprintf(
      stderr,
      "Pointer %p allocated with Arena::push_array is not aligned properly.\n",
      static_cast<void const*>(aligned));
}

static void test_rewind(yacsl::testing::Test& test) {
  {
    yacsl::memory::Arena::Scope const scope(
        yacsl::memory::MainArenaAllocator::get()); // test scope with ptr_ = 0
  }
  // Calls malloc so that the rewind test uses the same memory in and outside of
  // the scope.
  (void)yacsl::memory::MainArenaAllocator::get().push<int>();
  char const* ptr = nullptr;
  {
    yacsl::memory::Arena::Scope const scope(
        yacsl::memory::MainArenaAllocator::get());

    ptr = yacsl::memory::MainArenaAllocator::get().push_array<char>(10).data();

    test_align(test);
    test_ctors(test);
  }

  if (yacsl::memory::MainArenaAllocator::get().push_array<char>(1).data() !=
      ptr) {
    fprintf(stderr, "Arena didn't rewind to address %p.\n", ptr);
  }
}

int main() {
  int                           ret   = 0;
  yacsl::containers::StackArray tests = {
      yacsl::testing::Test("constructors", test_ctors),
      yacsl::testing::Test("pointer alignment", test_align),
      yacsl::testing::Test("rewinding", test_rewind),
  };
  for (auto& test : tests) {
    printf("Running test '%s'...\n", test.cstr_name());
    if (test.run_quietly() == yacsl::testing::Test::Result::Failure) {
      fprintf(stderr, "test '%s' failed.\n", test.cstr_name());
      ret = 1;
    }
  }
  return ret;
}
