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

static void test_constructor_with_len() noexcept {
  TestAllocator::HISTORY.clear();
  SMFTracker::HISTORY.clear();

  yacsl::containers::StackArray<char, 64> msg = {};

  {
    auto test3 = Vector<SMFTracker, 0, TestAllocator>(2);
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'allocator'\n"
                                          "copy constructed 'allocator'\n"
                                          "default constructed 'default'\n"
                                          "default constructed 'default'\n"
                                          "destroyed 'allocator'\n");
    snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
             sizeof(SMFTracker) * 2);
    EXPECT_STREQ(TestAllocator::HISTORY.ptr, msg.data());

    SMFTracker::HISTORY.clear();
    TestAllocator::HISTORY.clear();

    TestAllocator alloc;
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'allocator'\n");
    SMFTracker::HISTORY.clear();

    auto test4 = Vector<SMFTracker, 0, TestAllocator>(3, alloc);
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "copy constructed 'allocator'\n"
                                          "default constructed 'default'\n"
                                          "default constructed 'default'\n"
                                          "default constructed 'default'\n");
    snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
             sizeof(SMFTracker) * 3);
    EXPECT_STREQ(TestAllocator::HISTORY.ptr, msg.data());

    SMFTracker::HISTORY.clear();
    TestAllocator::HISTORY.clear();
  }
  EXPECT_STREQ(SMFTracker::HISTORY.ptr, "destroyed 'default'\n"
                                        "destroyed 'default'\n"
                                        "destroyed 'default'\n"
                                        "destroyed 'allocator (copy)'\n"
                                        "destroyed 'allocator'\n"
                                        "destroyed 'default'\n"
                                        "destroyed 'default'\n"
                                        "destroyed 'allocator (copy)'\n");
  snprintf(msg.data(), msg.len(),
           "deallocated %zu bytes\n"
           "deallocated %zu bytes\n",
           sizeof(SMFTracker) * 3, sizeof(SMFTracker) * 2);
  EXPECT_STREQ(TestAllocator::HISTORY.ptr, msg.data());
}

int main() {
  run_test("Vector([allocator])", test_constructor_with_nothing);
  run_test("Vector(len[, allocator])", test_constructor_with_len);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
