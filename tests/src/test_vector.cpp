#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "test_utils.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/testing/run.hpp"

static void test_allocator() {
  TestAllocator::clear_history();
  TestAllocator allocator;
  // NOLINTNEXTLINE(misc-const-correctness)
  auto mem = allocator.allocate(32, yacsl::memory::Alignment{16});

  EXPECT_STREQ(TestAllocator::HISTORY.ptr, "allocated 32 bytes\n");
  EXPECT(TestAllocator::POINTER_HIST.len == 1 &&
         TestAllocator::POINTER_HIST.ptr[0] == mem.data());
  TestAllocator::clear_history();

  allocator.deallocate(mem);

  EXPECT_STREQ(TestAllocator::HISTORY.ptr, "deallocated 32 bytes\n");
  EXPECT(TestAllocator::POINTER_HIST.len == 1 &&
         TestAllocator::POINTER_HIST.ptr[0] == mem.data());
}

int main() {
  fputs("TODO: proper vector tests\n", stderr);
  run_test("allocator", test_allocator);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
