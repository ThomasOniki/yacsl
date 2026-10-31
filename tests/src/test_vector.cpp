#include "test_utils.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/testing/run.hpp"
#include <stdio.h>
#include <stdlib.h>

using yacsl::containers::Vector;

static void test_constructor_with_nothing() noexcept {
  {
    auto test1 = Vector<SMFTracker, 0, TestAllocator>();
    EXPECT(test1.data() == nullptr);
    EXPECT(test1.len() == 0);
    EXPECT(test1.capacity() == 0);
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n");
    clear_logs();

    TestAllocator alloc;
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n");
    clear_logs();

    auto test2 = Vector<SMFTracker, 0, TestAllocator>(alloc);
    EXPECT(test2.data() == nullptr);
    EXPECT(test2.len() == 0);
    EXPECT(test2.capacity() == 0);
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'allocator'\n");
    clear_logs();
  }
  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'allocator (copy)'\n"
                                     "destroyed 'allocator'\n"
                                     "destroyed 'allocator'\n");
  EXPECT_STREQ(TestAllocator::LOGS.ptr, "deallocated 0 bytes\n"
                                        "deallocated 0 bytes\n");
}

static void test_constructor_with_len() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  {
    auto test1 = Vector<SMFTracker, 0, TestAllocator>(2);
    EXPECT(test1.len() == 2, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == 2, "capacity = %zu\n", test1.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "default constructed 'default'\n"
                                       "default constructed 'default'\n"
                                       "destroyed 'allocator'\n");
    snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
             sizeof(SMFTracker) * 2);
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();

    TestAllocator alloc;
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n");
    clear_logs();

    auto test2 = Vector<SMFTracker, 0, TestAllocator>(3, alloc);
    EXPECT(test2.len() == 3, "len = %zu.\n", test2.len());
    EXPECT(test2.capacity() == 3, "capacity = %zu\n", test2.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'allocator'\n"
                                       "default constructed 'default'\n"
                                       "default constructed 'default'\n"
                                       "default constructed 'default'\n");
    snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
             sizeof(SMFTracker) * 3);
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'default'\n"
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
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

int main() {
  run_test("Vector([allocator])", test_constructor_with_nothing);
  run_test("Vector(len[, allocator])", test_constructor_with_len);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
