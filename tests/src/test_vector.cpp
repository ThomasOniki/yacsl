#include "test_utils.hpp"
#include "yacsl/containers/stack_array.hpp"
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

template <yacsl::usize N> static void test_constructor_with_value() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  using yacsl::math::max;

  {
    auto value = SMFTracker("value");
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'value'\n");
    clear_logs();

    auto test1 = Vector<SMFTracker, N, TestAllocator>(2, value);
    EXPECT(test1.len() == 2, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == max(2UZ, N), "capacity = %zu\n",
           test1.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "copy constructed 'value'\n"
                                       "copy constructed 'value'\n"
                                       "destroyed 'allocator'\n");
    if (N < 2) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 2);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'value (copy)'\n"
                                     "destroyed 'value (copy)'\n"
                                     "destroyed 'allocator (copy)'\n"
                                     "destroyed 'value'\n");
  if (N < 2) {
    snprintf(msg.data(), msg.len(), "deallocated %zu bytes\n",
             sizeof(SMFTracker) * 2);
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

template <yacsl::usize N>
static void test_constructor_with_value_alloc() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  {
    auto          value = SMFTracker("value");
    TestAllocator alloc;
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'value'\n"
                                       "constructed 'allocator'\n");
    clear_logs();

    auto test1 = Vector<SMFTracker, N, TestAllocator>(3, value, alloc);
    EXPECT(test1.len() == 3, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == yacsl::math::max(3UZ, N), "capacity = %zu\n",
           test1.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'allocator'\n"
                                       "copy constructed 'value'\n"
                                       "copy constructed 'value'\n"
                                       "copy constructed 'value'\n");
    if (N < 3) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 3);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'value (copy)'\n"
                                     "destroyed 'value (copy)'\n"
                                     "destroyed 'value (copy)'\n"
                                     "destroyed 'allocator (copy)'\n"
                                     "destroyed 'allocator'\n"
                                     "destroyed 'value'\n");
  if (N < 3) {
    snprintf(msg.data(), msg.len(), "deallocated %zu bytes\n",
             sizeof(SMFTracker) * 3);
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

template <yacsl::usize N> static void test_constructor_with_init() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  using yacsl::math::max;

  {
    auto init = {
        SMFTracker("value1"),
        SMFTracker("value2"),
    };
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'value1'\n"
                                       "constructed 'value2'\n");
    clear_logs();

    auto test1 = Vector<SMFTracker, N, TestAllocator>(init);
    EXPECT(test1.len() == 2, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == max(2UZ, N), "capacity = %zu\n",
           test1.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "copy constructed 'value1'\n"
                                       "copy constructed 'value2'\n"
                                       "destroyed 'allocator'\n");
    if (N < 2) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 2);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'value2 (copy)'\n"
                                     "destroyed 'value1 (copy)'\n"
                                     "destroyed 'allocator (copy)'\n"
                                     "destroyed 'value2'\n"
                                     "destroyed 'value1'\n");
  if (N < 2) {
    snprintf(msg.data(), msg.len(), "deallocated %zu bytes\n",
             sizeof(SMFTracker) * 2);
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

template <yacsl::usize N>
static void test_constructor_with_init_alloc() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  {
    auto init = {
        SMFTracker("value1"),
        SMFTracker("value2"),
        SMFTracker("value3"),
        SMFTracker("value4"),
    };
    TestAllocator alloc;
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'value1'\n"
                                       "constructed 'value2'\n"
                                       "constructed 'value3'\n"
                                       "constructed 'value4'\n"
                                       "constructed 'allocator'\n");
    clear_logs();

    auto test1 = Vector<SMFTracker, N, TestAllocator>(init, alloc);
    EXPECT(test1.len() == 4, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == yacsl::math::max(4UZ, N), "capacity = %zu\n",
           test1.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'allocator'\n"
                                       "copy constructed 'value1'\n"
                                       "copy constructed 'value2'\n"
                                       "copy constructed 'value3'\n"
                                       "copy constructed 'value4'\n");
    if (N < 4) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 4);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'value4 (copy)'\n"
                                     "destroyed 'value3 (copy)'\n"
                                     "destroyed 'value2 (copy)'\n"
                                     "destroyed 'value1 (copy)'\n"
                                     "destroyed 'allocator (copy)'\n"
                                     "destroyed 'allocator'\n"
                                     "destroyed 'value4'\n"
                                     "destroyed 'value3'\n"
                                     "destroyed 'value2'\n"
                                     "destroyed 'value1'\n");
  if (N < 4) {
    snprintf(msg.data(), msg.len(), "deallocated %zu bytes\n",
             sizeof(SMFTracker) * 4);
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

// NOLINTBEGIN(bugprone-macro-parentheses)
#define RUN8(name, func)                                                       \
  run_test("Vector<T, 0>::" name, func<0>);                                    \
  run_test("Vector<T, 1>::" name, func<1>);                                    \
  run_test("Vector<T, 2>::" name, func<2>);                                    \
  run_test("Vector<T, 3>::" name, func<3>);                                    \
  run_test("Vector<T, 4>::" name, func<4>);                                    \
  run_test("Vector<T, 5>::" name, func<5>);                                    \
  run_test("Vector<T, 6>::" name, func<6>);                                    \
  run_test("Vector<T, 7>::" name, func<7>);
// NOLINTEND(bugprone-macro-parentheses)

int main() {
  RUN8("Vector()", test_constructor_with_nothing);
  RUN8("Vector(allocator)", test_constructor_with_alloc);
  RUN8("Vector(len)", test_constructor_with_len);
  RUN8("Vector(len, allocator)", test_constructor_with_len_alloc);
  RUN8("Vector(len, value)", test_constructor_with_value);
  RUN8("Vector(len, value, allocator)", test_constructor_with_value_alloc);
  RUN8("Vector(init)", test_constructor_with_init);
  RUN8("Vector(init, allocator)", test_constructor_with_init_alloc);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
