#include "test_utils.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/testing/run.hpp"
#include <stdio.h>
#include <stdlib.h>

using yacsl::containers::Vector;

static void test_constructor_with_nothing() noexcept {
  SMFTracker::HISTORY.clear();
  TestAllocator::HISTORY.clear();

  {
    auto test1 = Vector<SMFTracker, 0, TestAllocator>();
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'allocator'\n");
    SMFTracker::HISTORY.clear();

    TestAllocator alloc;
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'allocator'\n");
    SMFTracker::HISTORY.clear();

    auto test2 = Vector<SMFTracker, 0, TestAllocator>(alloc);
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "copy constructed 'allocator'\n");
    SMFTracker::HISTORY.clear();
  }
  EXPECT_STREQ(SMFTracker::HISTORY.ptr, "destroyed 'allocator (copy)'\n"
                                        "destroyed 'allocator'\n"
                                        "destroyed 'allocator'\n");
  EXPECT_STREQ(TestAllocator::HISTORY.ptr, "deallocated 0 bytes\n"
                                           "deallocated 0 bytes\n");
}

int main() {
  run_test("Vector([allocator])", test_constructor_with_nothing);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
