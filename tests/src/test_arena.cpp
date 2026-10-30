#include "test_utils.hpp"
#include "yacsl/memory/arena.hpp"
#include "yacsl/testing/run.hpp"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

constexpr uint32_t iteration_count = 10000;

namespace {
struct CtorCounter {
  inline static uint32_t ctor_call_count = 0;
  explicit CtorCounter(uint32_t num) : member(num) { ++ctor_call_count; }
  [[maybe_unused]] CtorCounter() { ++ctor_call_count; }
  uint32_t member = 0;
};

struct Aligned {
  alignas(64) char data;
};
} // namespace

static void test_ctors() {
  CtorCounter::ctor_call_count = 0;

  CtorCounter* ptrs[iteration_count] = {};
  for (uint32_t i = 0; i < iteration_count; ++i) {
    ptrs[i] = &yacsl::memory::MainArenaAllocator::get().push<CtorCounter>(i);
  }

  EXPECT(CtorCounter::ctor_call_count == iteration_count,
         "%u constructors were called.\n", CtorCounter::ctor_call_count);

  for (uint32_t i = 0; i < iteration_count; ++i) {
    EXPECT(ptrs[i]->member == i, "The actual value was %u\n", ptrs[i]->member);
  }

  CtorCounter::ctor_call_count = 0;

  for (auto& ptr : ptrs) {
    ptr = yacsl::memory::MainArenaAllocator::get()
              .push_array<CtorCounter>(10)
              .data();
  }

  EXPECT(CtorCounter::ctor_call_count == iteration_count * 10,
         "%u constructors were called.\n", CtorCounter::ctor_call_count);

  for (auto const* ptr : ptrs) {
    for (uint32_t j = 0; j < 10; ++j) {
      EXPECT(ptr[j].member == 0, "The actual value was %u\n", ptr[j].member);
    }
  }
}

static bool is_aligned(Aligned const* ptr) {
  auto const uptr = reinterpret_cast<uintptr_t>(ptr);
  return uptr % alignof(Aligned) == 0;
}

static void test_align() {
  (void)yacsl::memory::MainArenaAllocator::get().push_array<char>(15);

  Aligned const* aligned =
      &yacsl::memory::MainArenaAllocator::get().push<Aligned>();

  EXPECT(is_aligned(aligned));

  (void)yacsl::memory::MainArenaAllocator::get().push_array<char>(15);

  aligned =
      yacsl::memory::MainArenaAllocator::get().push_array<Aligned>(10).data();

  EXPECT(is_aligned(aligned));
}

static void test_rewind() {
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

    test_align();
    test_ctors();
  }

  EXPECT(yacsl::memory::MainArenaAllocator::get().push_array<char>(1).data() ==
             ptr,
         "Arena didn't rewind.\n");
}

int main() {
  run_test("constructors", test_ctors);
  run_test("pointer alignment", test_align);
  run_test("rewinding", test_rewind);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
