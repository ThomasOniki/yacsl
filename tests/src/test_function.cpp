#include "test_utils.hpp"
#include "yacsl/containers/function.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/testing/run.hpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void test_function_runs() {
  bool                                ran = false;
  yacsl::containers::Function<void()> func([&ran] { ran = true; });

  EXPECT(!ran);
  func();
  EXPECT(ran);
}

static void test_moved_lambda() {
  {
    // test must be non-to const be moved.
    // NOLINTNEXTLINE(misc-const-correctness)
    auto test1 = SMFTracker("test1");

    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'test1'\n");
    clear_logs();

    auto lambda = [test1](int value) { return value * 2; };

    EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'test1'\n");
    clear_logs();

    EXPECT_STREQ(TestAllocator::LOGS.ptr, "");

    yacsl::containers::Function<int(int), 0, TestAllocator> const func =
        yacsl::memory::move(lambda);

    char buf[64];
    snprintf(buf, 64, "allocated %zu bytes\n", sizeof(SMFTracker));
    EXPECT_STREQ(TestAllocator::LOGS.ptr, buf);
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n"
                                       "move constructed 'test1 (copy)'\n");
    clear_logs();

    EXPECT(func(10) == 20);
  }
  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'test1 (copy) (moved)'\n"
                                     "destroyed 'allocator'\n"
                                     "destroyed 'test1 (copy)'\n"
                                     "destroyed 'test1'\n");

  char buf[64];
  snprintf(buf, 64, "deallocated %zu bytes\n", sizeof(SMFTracker));
  EXPECT_STREQ(TestAllocator::LOGS.ptr, buf);
  clear_logs();
}

static void test_copied_lambda() {
  {
    auto const test2 = SMFTracker("test2");
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'test2'\n");
    clear_logs();

    auto lambda = [test2] { (void)test2; };
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'test2'\n");
    clear_logs();

    EXPECT_STREQ(TestAllocator::LOGS.ptr, "");

    yacsl::containers::Function<void(), sizeof(SMFTracker), TestAllocator> const
        func = lambda;

    EXPECT_STREQ(TestAllocator::LOGS.ptr, "");

    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n"
                                       "copy constructed 'test2 (copy)'\n");
    clear_logs();
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, "");

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'test2 (copy) (copy)'\n"
                                     "destroyed 'allocator'\n"
                                     "destroyed 'test2 (copy)'\n"
                                     "destroyed 'test2'\n");
}

static void test_ref_capture_lamdba() {
  {
    auto test3 = SMFTracker("test3");
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'test3'\n");
    clear_logs();

    auto lambda = [&test3] { (void)test3; };
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "");

    yacsl::containers::Function<void()> const func(lambda);
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "");
  }
  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'test3'\n");
}

int main() {
  run_test("function runs", test_function_runs);
  run_test("moved lambda", test_moved_lambda);
  run_test("copied lambda", test_copied_lambda);
  run_test("ref capture lambda", test_ref_capture_lamdba);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
