#include "test_utils.hpp"
#include "yacsl/containers/function.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/testing/expect.hpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void test_function_runs() {
  bool                                ran = false;
  yacsl::containers::Function<void()> func([&ran] { ran = true; });

  yacsl::testing::expect(!ran);
  func();
  yacsl::testing::expect(ran);
}

static void test_moved_lambda() {
  SMFTracker::HISTORY.clear();
  TestAllocator::HISTORY.clear();
  {
    // test must be non-to const be moved.
    // NOLINTNEXTLINE(misc-const-correctness)
    auto test1 = SMFTracker("test1");

    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'test1'\n");
    SMFTracker::HISTORY.clear();

    auto lambda = [test1](int value) { return value * 2; };

    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "copy constructed 'test1'\n");
    SMFTracker::HISTORY.clear();

    EXPECT_STREQ(TestAllocator::HISTORY.ptr, "");

    yacsl::containers::Function<int(int), 0, TestAllocator> const func =
        yacsl::memory::move(lambda);

    char buf[64];
    snprintf(buf, 64, "allocated %zu bytes\n", sizeof(SMFTracker));
    EXPECT_STREQ(TestAllocator::HISTORY.ptr, buf);
    TestAllocator::HISTORY.clear();

    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'allocator'\n"
                                          "move constructed 'test1 (copy)'\n");
    SMFTracker::HISTORY.clear();

    EXPECT(func(10) == 20);
  }
  EXPECT_STREQ(SMFTracker::HISTORY.ptr, "destroyed 'test1 (copy) (moved)'\n"
                                        "destroyed 'allocator'\n"
                                        "destroyed 'test1 (copy)'\n"
                                        "destroyed 'test1'\n");

  char buf[64];
  snprintf(buf, 64, "deallocated %zu bytes\n", sizeof(SMFTracker));
  EXPECT_STREQ(TestAllocator::HISTORY.ptr, buf);
  TestAllocator::HISTORY.clear();
}

static void test_copied_lambda() {
  SMFTracker::HISTORY.clear();
  TestAllocator::HISTORY.clear();
  {
    auto const test2 = SMFTracker("test2");
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'test2'\n");
    SMFTracker::HISTORY.clear();

    auto lambda = [test2] { (void)test2; };
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "copy constructed 'test2'\n");
    SMFTracker::HISTORY.clear();

    EXPECT_STREQ(TestAllocator::HISTORY.ptr, "");

    yacsl::containers::Function<void(), sizeof(SMFTracker), TestAllocator> const
        func = lambda;

    EXPECT_STREQ(TestAllocator::HISTORY.ptr, "");

    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'allocator'\n"
                                          "copy constructed 'test2 (copy)'\n");
    SMFTracker::HISTORY.clear();
  }
  EXPECT_STREQ(TestAllocator::HISTORY.ptr, "");

  EXPECT_STREQ(SMFTracker::HISTORY.ptr, "destroyed 'test2 (copy) (copy)'\n"
                                        "destroyed 'allocator'\n"
                                        "destroyed 'test2 (copy)'\n"
                                        "destroyed 'test2'\n");
}

static void test_ref_capture_lamdba() {
  {
    SMFTracker::HISTORY.clear();

    auto test3 = SMFTracker("test3");
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "constructed 'test3'\n");
    SMFTracker::HISTORY.clear();

    auto lambda = [&test3] { (void)test3; };
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "");

    yacsl::containers::Function<void()> const func(lambda);
    EXPECT_STREQ(SMFTracker::HISTORY.ptr, "");
  }
  EXPECT_STREQ(SMFTracker::HISTORY.ptr, "destroyed 'test3'\n");
}

int main() {
  yacsl::testing::run("function runs", test_function_runs);
  yacsl::testing::run("moved lambda", test_moved_lambda);
  yacsl::testing::run("copied lambda", test_copied_lambda);
  yacsl::testing::run("ref capture lambda", test_ref_capture_lamdba);
  yacsl::debug::flush_logger();
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
