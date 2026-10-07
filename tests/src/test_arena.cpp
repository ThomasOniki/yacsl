#include "test_utils.hpp"
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
  explicit CtorCounter(uint32_t num) : member(num) { ++ctor_call_count; }
  [[maybe_unused]] CtorCounter() { ++ctor_call_count; }
  uint32_t member = 0;
};

struct Aligned {
  alignas(64) char data;
};
} // namespace

static void test_ctors(yacsl::testing::Test& /*test*/) {
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

static void test_align(yacsl::testing::Test& /*test*/) {
  (void)yacsl::memory::MainArenaAllocator::get().push_array<char>(15);

  Aligned const* aligned =
      &yacsl::memory::MainArenaAllocator::get().push<Aligned>();

  EXPECT(is_aligned(aligned));

  (void)yacsl::memory::MainArenaAllocator::get().push_array<char>(15);

  aligned =
      yacsl::memory::MainArenaAllocator::get().push_array<Aligned>(10).data();

  EXPECT(is_aligned(aligned));
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

  EXPECT(yacsl::memory::MainArenaAllocator::get().push_array<char>(1).data() ==
             ptr,
         "Arena didn't rewind.\n");
}

int main() {
  yacsl::containers::StackArray tests = {
      yacsl::testing::Test("constructors", test_ctors),
      yacsl::testing::Test("pointer alignment", test_align),
      yacsl::testing::Test("rewinding", test_rewind),
  };
  return run_tests(tests.slice());
}
