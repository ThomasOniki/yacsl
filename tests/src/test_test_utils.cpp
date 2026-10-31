#include "test_utils.hpp"

static void test_allocator() {
  clear_logs();
  TestAllocator allocator;
  // NOLINTNEXTLINE(misc-const-correctness)
  auto mem = allocator.allocate(32, yacsl::memory::Alignment{16});

  EXPECT_STREQ(TestAllocator::LOGS.ptr, "allocated 32 bytes\n");
  EXPECT(TestAllocator::PTR_LOGS.len == 1 &&
         TestAllocator::PTR_LOGS.ptr[0] == mem.data());
  clear_logs();

  allocator.deallocate(mem);

  EXPECT_STREQ(TestAllocator::LOGS.ptr, "deallocated 32 bytes\n");
  EXPECT(TestAllocator::PTR_LOGS.len == 1 &&
         TestAllocator::PTR_LOGS.ptr[0] == mem.data());
}

int main() {
  run_test("allocator", test_allocator);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
