#include "test_utils.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/math/arithmetic.hpp"
#include "yacsl/ranges/initializer_list.hpp"
#include "yacsl/testing/run.hpp"
#include "yacsl/type_aliases.hpp"
#include <stddef.h>
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

template <yacsl::usize N, typename R>
static void test_constructor_with_range_impl(R const& range) noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  using yacsl::math::max;

  clear_logs();

  {
    Vector<SMFTracker, N, TestAllocator> test1 = range;
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
                                     "destroyed 'allocator (copy)'\n");
  if (N < 2) {
    snprintf(msg.data(), msg.len(), "deallocated %zu bytes\n",
             sizeof(SMFTracker) * 2);
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

  [[maybe_unused]] Vector<SMFTracker, N, TestAllocator> test2 = {
      SMFTracker("This wouldn't compile if there was no overload for "),
      SMFTracker("initializer_list, only the generic Vector(RangeOf<T>)."),
  };
}

template <yacsl::usize N, typename R, bool Reverse = false>
static void test_constructor_with_range() noexcept {
  if (Reverse) {
    test_constructor_with_range_impl<N, R>({
        SMFTracker("value2"),
        SMFTracker("value1"),
    });
  } else {
    test_constructor_with_range_impl<N, R>({
        SMFTracker("value1"),
        SMFTracker("value2"),
    });
  }
}

template <yacsl::usize N, typename R>
static void test_constructor_with_range_alloc_impl(R const& range) noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  using yacsl::math::max;

  clear_logs();

  {
    TestAllocator alloc;
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'allocator'\n");
    clear_logs();

    Vector<SMFTracker, N, TestAllocator> test1 = {range, alloc};
    EXPECT(test1.len() == 2, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == max(2UZ, N), "capacity = %zu\n",
           test1.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'allocator'\n"
                                       "copy constructed 'value1'\n"
                                       "copy constructed 'value2'\n");
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
                                     "destroyed 'allocator'\n");
  if (N < 2) {
    snprintf(msg.data(), msg.len(), "deallocated %zu bytes\n",
             sizeof(SMFTracker) * 2);
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

  [[maybe_unused]] Vector<SMFTracker, N, TestAllocator> test2 = {
      SMFTracker("This wouldn't compile if there was no overload for "),
      SMFTracker("initializer_list, only the generic Vector(RangeOf<T>)."),
  };
}

template <yacsl::usize N, typename R, bool Reverse = false>
static void test_constructor_with_range_alloc() noexcept {
  if (Reverse) {
    test_constructor_with_range_alloc_impl<N, R>({
        SMFTracker("value2"),
        SMFTracker("value1"),
    });
  } else {
    test_constructor_with_range_alloc_impl<N, R>({
        SMFTracker("value1"),
        SMFTracker("value2"),
    });
  }
}

template <yacsl::usize N> static void test_constructor_with_copy() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  using yacsl::math::max;

  {
    auto other = Vector<SMFTracker, N, TestAllocator>{
        SMFTracker("value1"),
        SMFTracker("value2"),
    };
    EXPECT(other.len() == 2, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == max(2UZ, N), "capacity = %zu\n",
           other.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'value1'\n"
                                       "constructed 'value2'\n"
                                       "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "copy constructed 'value1'\n"
                                       "copy constructed 'value2'\n"
                                       "destroyed 'allocator'\n"
                                       "destroyed 'value2'\n"
                                       "destroyed 'value1'\n");
    if (N < 2) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 2);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
    clear_logs();

    auto test1 = Vector<SMFTracker, N, TestAllocator>(other);
    EXPECT(test1.len() == 2, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == max(2UZ, N), "capacity = %zu\n",
           test1.capacity());
    EXPECT(other.len() == 2, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == max(2UZ, N), "capacity = %zu\n",
           other.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'allocator (copy)'\n"
                                       "copy constructed 'value1 (copy)'\n"
                                       "copy constructed 'value2 (copy)'\n");
    if (N < 2) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 2);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'value2 (copy) (copy)'\n"
                                     "destroyed 'value1 (copy) (copy)'\n"
                                     "destroyed 'allocator (copy) (copy)'\n"
                                     "destroyed 'value2 (copy)'\n"
                                     "destroyed 'value1 (copy)'\n"
                                     "destroyed 'allocator (copy)'\n");
  if (N < 2) {
    snprintf(msg.data(), msg.len(),
             "deallocated %zu bytes\n"
             "deallocated %zu bytes\n",
             sizeof(SMFTracker) * 2, sizeof(SMFTracker) * 2);
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

template <yacsl::usize N> static void test_constructor_with_move() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  using yacsl::math::max;

  {
    Vector<SMFTracker, N, TestAllocator> other = {
        SMFTracker("value1"),
        SMFTracker("value2"),
        SMFTracker("value3"),
    };
    EXPECT(other.len() == 3, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == max(3UZ, N), "capacity = %zu\n",
           other.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'value1'\n"
                                       "constructed 'value2'\n"
                                       "constructed 'value3'\n"
                                       "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "copy constructed 'value1'\n"
                                       "copy constructed 'value2'\n"
                                       "copy constructed 'value3'\n"
                                       "destroyed 'allocator'\n"
                                       "destroyed 'value3'\n"
                                       "destroyed 'value2'\n"
                                       "destroyed 'value1'\n");
    if (N < 3) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 3);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
    clear_logs();

    auto test1 = Vector(yacsl::memory::move(other));
    EXPECT(test1.len() == 3, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == max(3UZ, N), "capacity = %zu\n",
           test1.capacity());
    EXPECT(other.len() == 0, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == N, "capacity = %zu\n", other.capacity());
    if (N < 3) {
      EXPECT_STREQ(SMFTracker::LOGS.ptr,
                   "move constructed 'allocator (copy)'\n");
    } else {
      EXPECT_STREQ(SMFTracker::LOGS.ptr, "move constructed 'allocator (copy)'\n"
                                         "move constructed 'value1 (copy)'\n"
                                         "move constructed 'value2 (copy)'\n"
                                         "move constructed 'value3 (copy)'\n"
                                         "destroyed 'value3 (copy)'\n"
                                         "destroyed 'value2 (copy)'\n"
                                         "destroyed 'value1 (copy)'\n");
    }
    msg[0] = '\0';
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  if (N < 3) {
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'value3 (copy)'\n"
                                       "destroyed 'value2 (copy)'\n"
                                       "destroyed 'value1 (copy)'\n"
                                       "destroyed 'allocator (copy) (moved)'\n"
                                       "destroyed 'allocator (copy)'\n");
    if (N > 0) {
      // If N>0, after being moved, other.data_ will point to its stack buffer
      // and there is a check to make sure the stack buffer isn't freed.
      snprintf(msg.data(), msg.len(), "deallocated %zu bytes\n",
               sizeof(SMFTracker) * 3);
    } else {
      snprintf(msg.data(), msg.len(),
               "deallocated %zu bytes\n"
               "deallocated 0 bytes\n",
               sizeof(SMFTracker) * 3);
    }
  } else {
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'value3 (copy) (moved)'\n"
                                       "destroyed 'value2 (copy) (moved)'\n"
                                       "destroyed 'value1 (copy) (moved)'\n"
                                       "destroyed 'allocator (copy) (moved)'\n"
                                       "destroyed 'allocator (copy)'\n");
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

template <yacsl::usize N>
static void test_constructor_with_copy_alloc() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  using yacsl::math::max;

  {
    auto other = Vector<SMFTracker, N, TestAllocator>{
        SMFTracker("value1"),
        SMFTracker("value2"),
    };
    TestAllocator alloc;

    EXPECT(other.len() == 2, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == max(2UZ, N), "capacity = %zu\n",
           other.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'value1'\n"
                                       "constructed 'value2'\n"
                                       "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "copy constructed 'value1'\n"
                                       "copy constructed 'value2'\n"
                                       "destroyed 'allocator'\n"
                                       "destroyed 'value2'\n"
                                       "destroyed 'value1'\n"
                                       "constructed 'allocator'\n");
    if (N < 2) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 2);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
    clear_logs();

    auto test1 = Vector<SMFTracker, N, TestAllocator>(other, alloc);
    EXPECT(test1.len() == 2, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == max(2UZ, N), "capacity = %zu\n",
           test1.capacity());
    EXPECT(other.len() == 2, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == max(2UZ, N), "capacity = %zu\n",
           other.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'allocator'\n"
                                       "copy constructed 'value1 (copy)'\n"
                                       "copy constructed 'value2 (copy)'\n");
    if (N < 2) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 2);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'value2 (copy) (copy)'\n"
                                     "destroyed 'value1 (copy) (copy)'\n"
                                     "destroyed 'allocator (copy)'\n"
                                     "destroyed 'allocator'\n"
                                     "destroyed 'value2 (copy)'\n"
                                     "destroyed 'value1 (copy)'\n"
                                     "destroyed 'allocator (copy)'\n");
  if (N < 2) {
    snprintf(msg.data(), msg.len(),
             "deallocated %zu bytes\n"
             "deallocated %zu bytes\n",
             sizeof(SMFTracker) * 2, sizeof(SMFTracker) * 2);
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

template <yacsl::usize N>
static void test_constructor_with_move_alloc() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  using yacsl::math::max;

  {
    Vector<SMFTracker, N, TestAllocator> other = {
        SMFTracker("value1"),
        SMFTracker("value2"),
        SMFTracker("value3"),
    };
    TestAllocator alloc;

    EXPECT(other.len() == 3, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == max(3UZ, N), "capacity = %zu\n",
           other.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed 'value1'\n"
                                       "constructed 'value2'\n"
                                       "constructed 'value3'\n"
                                       "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "copy constructed 'value1'\n"
                                       "copy constructed 'value2'\n"
                                       "copy constructed 'value3'\n"
                                       "destroyed 'allocator'\n"
                                       "destroyed 'value3'\n"
                                       "destroyed 'value2'\n"
                                       "destroyed 'value1'\n"
                                       "constructed 'allocator'\n");
    if (N < 3) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 3);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
    clear_logs();

    auto test1 = Vector(yacsl::memory::move(other), alloc);
    EXPECT(test1.len() == 3, "len = %zu.\n", test1.len());
    EXPECT(test1.capacity() == max(3UZ, N), "capacity = %zu\n",
           test1.capacity());
    EXPECT(other.len() == 0, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == N, "capacity = %zu\n", other.capacity());
    if (N < 3) {
      EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'allocator'\n");
    } else {
      EXPECT_STREQ(SMFTracker::LOGS.ptr, "copy constructed 'allocator'\n"
                                         "move constructed 'value1 (copy)'\n"
                                         "move constructed 'value2 (copy)'\n"
                                         "move constructed 'value3 (copy)'\n"
                                         "destroyed 'value3 (copy)'\n"
                                         "destroyed 'value2 (copy)'\n"
                                         "destroyed 'value1 (copy)'\n");
    }
    msg[0] = '\0';
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  if (N < 3) {
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'value3 (copy)'\n"
                                       "destroyed 'value2 (copy)'\n"
                                       "destroyed 'value1 (copy)'\n"
                                       "destroyed 'allocator (copy)'\n"
                                       "destroyed 'allocator'\n"
                                       "destroyed 'allocator (copy)'\n");
    if (N > 0) {
      // If N>0, after being moved, other.data_ will point to its stack buffer
      // and there is a check to make sure the stack buffer isn't freed.
      snprintf(msg.data(), msg.len(), "deallocated %zu bytes\n",
               sizeof(SMFTracker) * 3);
    } else {
      snprintf(msg.data(), msg.len(),
               "deallocated %zu bytes\n"
               "deallocated 0 bytes\n",
               sizeof(SMFTracker) * 3);
    }
  } else {
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed 'value3 (copy) (moved)'\n"
                                       "destroyed 'value2 (copy) (moved)'\n"
                                       "destroyed 'value1 (copy) (moved)'\n"
                                       "destroyed 'allocator (copy)'\n"
                                       "destroyed 'allocator'\n"
                                       "destroyed 'allocator (copy)'\n");
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

// NOLINTBEGIN(bugprone-macro-parentheses)
#define RUN8(name, func, ...)                                                  \
  run_test("Vector<T, 0>::" name, func<0 __VA_OPT__(, ) __VA_ARGS__>);         \
  run_test("Vector<T, 1>::" name, func<1 __VA_OPT__(, ) __VA_ARGS__>);         \
  run_test("Vector<T, 2>::" name, func<2 __VA_OPT__(, ) __VA_ARGS__>);         \
  run_test("Vector<T, 3>::" name, func<3 __VA_OPT__(, ) __VA_ARGS__>);         \
  run_test("Vector<T, 4>::" name, func<4 __VA_OPT__(, ) __VA_ARGS__>);         \
  run_test("Vector<T, 5>::" name, func<5 __VA_OPT__(, ) __VA_ARGS__>);         \
  run_test("Vector<T, 6>::" name, func<6 __VA_OPT__(, ) __VA_ARGS__>);         \
  run_test("Vector<T, 7>::" name, func<7 __VA_OPT__(, ) __VA_ARGS__>);
// NOLINTEND(bugprone-macro-parentheses)

int main() {
  RUN8("Vector()", test_constructor_with_nothing);
  RUN8("Vector(allocator)", test_constructor_with_alloc);
  RUN8("Vector(len)", test_constructor_with_len);
  RUN8("Vector(len, allocator)", test_constructor_with_len_alloc);
  RUN8("Vector(len, value)", test_constructor_with_value);
  RUN8("Vector(len, value, allocator)", test_constructor_with_value_alloc);

  RUN8("Vector(init)", test_constructor_with_range,
       yacsl::ranges::InitializerList<SMFTracker>);
  RUN8("Vector(stack_array)", test_constructor_with_range,
       yacsl::containers::StackArray<SMFTracker, 2>);
  RUN8("Vector(slice)", test_constructor_with_range,
       yacsl::ranges::Slice<SMFTracker const>);
  RUN8("Vector(reversed_slice)", test_constructor_with_range,
       yacsl::ranges::ReversedSlice<SMFTracker const>, true);

  RUN8("Vector(init, allocator)", test_constructor_with_range_alloc,
       yacsl::ranges::InitializerList<SMFTracker>);
  RUN8("Vector(stack_array, allocator)", test_constructor_with_range_alloc,
       yacsl::containers::StackArray<SMFTracker, 2>);
  RUN8("Vector(slice, allocator)", test_constructor_with_range_alloc,
       yacsl::ranges::Slice<SMFTracker const>);
  RUN8("Vector(reversed_slice, allocator)", test_constructor_with_range_alloc,
       yacsl::ranges::ReversedSlice<SMFTracker const>, true);

  RUN8("Vector(other)", test_constructor_with_copy);
  RUN8("Vector(move(other))", test_constructor_with_move);
  RUN8("Vector(other, allocator)", test_constructor_with_copy_alloc);
  RUN8("Vector(move(other), allocator)", test_constructor_with_move_alloc);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
