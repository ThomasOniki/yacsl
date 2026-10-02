#include "test_utils.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/memory/arena.hpp"
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

static int test_ctors() {
  int ret                      = 0;
  CtorCounter::ctor_call_count = 0;

  CtorCounter* ptrs[iteration_count] = {};
  for (uint32_t i = 0; i < iteration_count; ++i) {
    ptrs[i] = &yacsl::memory::MainArenaAllocator::get().push<CtorCounter>(i);
  }

  EXPECT_EQ(CtorCounter::ctor_call_count, iteration_count);

  for (uint32_t i = 0; i < iteration_count; ++i) {
    EXPECT_EQ(ptrs[i]->i_, i);
  }

  CtorCounter::ctor_call_count = 0;

  for (auto& ptr : ptrs) {
    ptr = yacsl::memory::MainArenaAllocator::get()
              .push_array<CtorCounter>(10)
              .data();
  }

  EXPECT_EQ(CtorCounter::ctor_call_count, iteration_count * 10);

  for (auto const& ptr : ptrs) {
    for (uint32_t i = 0; i < 10; ++i) {
      EXPECT_EQ(ptr->i_, 0U);
    }
  }
  return ret;
}

static bool is_aligned(Aligned const* ptr) {
  auto const uptr = reinterpret_cast<uintptr_t>(ptr);
  return uptr % alignof(Aligned) == 0;
}

static int test_align() {
  int ret = 0;
  (void)yacsl::memory::MainArenaAllocator::get().push_array<char>(15);

  Aligned const* aligned =
      &yacsl::memory::MainArenaAllocator::get().push<Aligned>();

  EXPECT(is_aligned(aligned));

  (void)yacsl::memory::MainArenaAllocator::get().push_array<char>(15);

  aligned =
      yacsl::memory::MainArenaAllocator::get().push_array<Aligned>(10).data();

  EXPECT(is_aligned(aligned));

  return ret;
}

static int test_rewind() {
  int ret = 0;
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
             ptr &&
         "memory::Arena didn't rewind");
  return ret;
}

int main() {
  int ret = 0;
  EXPECT(test_align() == 0);
  EXPECT(test_ctors() == 0);
  EXPECT(test_rewind() == 0);
  yacsl::debug::Logger::get_default_instance().flush();
  return ret;
}
