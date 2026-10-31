#include "test_utils.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/math/arithmetic.hpp"
#include "yacsl/testing/run.hpp"
#include "yacsl/type_aliases.hpp"
#include <stdio.h>
#include <stdlib.h>

using yacsl::containers::Vector;

template <yacsl::usize N> static void test_constructor_with_nothing() noexcept {
  {
    auto test1 = Vector<SMFTracker, N, TestAllocator>();
    EXPECT(N > 0 || test1.data() == nullptr);
    EXPECT(test1.len() == 0);
    EXPECT(test1.capacity() == N);
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n");
    clear_logs();
  }
  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'allocator'\n");
  EXPECT_STREQ(TestAllocator::LOGS.ptr, N > 0 ? "" : "deallocated 0 bytes\n");
}

template <yacsl::usize N> static void test_constructor_with_alloc() noexcept {
  {
    TestAllocator alloc;
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n");
    clear_logs();

    auto test1 = Vector<SMFTracker, N, TestAllocator>(alloc);
    EXPECT(N > 0 || test1.data() == nullptr);
    EXPECT(test1.len() == 0);
    EXPECT(test1.capacity() == N);
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'allocator'\n");
    clear_logs();
  }
  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'allocator (copy)'\n"
                                     "destroyed 'allocator'\n");
  EXPECT_STREQ(TestAllocator::LOGS.ptr, N > 0 ? "" : "deallocated 0 bytes\n");
}

template <yacsl::usize N> static void test_constructor_with_len() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  using yacsl::math::max;

  {
    auto test1 = Vector<SMFTracker, N, TestAllocator>(2);
    EXPECT(test1.len() == 2, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == max(2UZ, N), "capacity = %zu\n",
           test1.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "default constructed 'default'\n"
                                       "default constructed 'default'\n"
                                       "destroyed 'allocator'\n");
    if (N < 2) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 2);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'default'\n"
                                     "destroyed 'default'\n"
                                     "destroyed 'allocator (copy)'\n");
  if (N < 2) {
    snprintf(msg.data(), msg.len(), "deallocated %zu bytes\n",
             sizeof(SMFTracker) * 2);
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

template <yacsl::usize N>
static void test_constructor_with_len_alloc() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  {
    TestAllocator alloc;
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n");
    clear_logs();

    auto test1 = Vector<SMFTracker, 0, TestAllocator>(3, alloc);
    EXPECT(test1.len() == 3, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == 3, "capacity = %zu\n", test1.capacity());
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
                                     "destroyed 'allocator'\n");
  snprintf(msg.data(), msg.len(), "deallocated %zu bytes\n",
           sizeof(SMFTracker) * 3);
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

int main() {
  run_test("Vector<T, 0>::Vector()", test_constructor_with_nothing<0>);
  run_test("Vector<T, 5>::Vector()", test_constructor_with_nothing<5>);
  run_test("Vector<T, 0>::Vector(allocator)", test_constructor_with_alloc<0>);
  run_test("Vector<T, 5>::Vector(allocator)", test_constructor_with_alloc<5>);
  run_test("Vector<T, 0>::Vector(len)", test_constructor_with_len<0>);
  run_test("Vector<T, 1>::Vector(len)", test_constructor_with_len<1>);
  run_test("Vector<T, 2>::Vector(len)", test_constructor_with_len<2>);
  run_test("Vector<T, 3>::Vector(len)", test_constructor_with_len<3>);
  run_test("Vector<T, 0>::Vector(len, allocator)",
           test_constructor_with_len_alloc<0>);
  run_test("Vector<T, 3>::Vector(len, allocator)",
           test_constructor_with_len_alloc<3>);
  run_test("Vector<T, 4>::Vector(len, allocator)",
           test_constructor_with_len_alloc<4>);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
