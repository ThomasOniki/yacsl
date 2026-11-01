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
                                       "copy constructed '1'\n"
                                       "copy constructed '2'\n"
                                       "destroyed 'allocator'\n");
    if (N < 2) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 2);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed '2 (copy)'\n"
                                     "destroyed '1 (copy)'\n"
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
        SMFTracker("2"),
        SMFTracker("1"),
    });
  } else {
    test_constructor_with_range_impl<N, R>({
        SMFTracker("1"),
        SMFTracker("2"),
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
                                       "copy constructed '1'\n"
                                       "copy constructed '2'\n");
    if (N < 2) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 2);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed '2 (copy)'\n"
                                     "destroyed '1 (copy)'\n"
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
        SMFTracker("2"),
        SMFTracker("1"),
    });
  } else {
    test_constructor_with_range_alloc_impl<N, R>({
        SMFTracker("1"),
        SMFTracker("2"),
    });
  }
}

template <yacsl::usize N> static void test_constructor_with_copy() noexcept {
  yacsl::containers::StackArray<char, 64> msg = {};

  using yacsl::math::max;

  {
    auto other = Vector<SMFTracker, N, TestAllocator>{
        SMFTracker("1"),
        SMFTracker("2"),
    };
    EXPECT(other.len() == 2, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == max(2UZ, N), "capacity = %zu\n",
           other.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed '1'\n"
                                       "constructed '2'\n"
                                       "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "copy constructed '1'\n"
                                       "copy constructed '2'\n"
                                       "destroyed 'allocator'\n"
                                       "destroyed '2'\n"
                                       "destroyed '1'\n");
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
                                       "copy constructed '1 (copy)'\n"
                                       "copy constructed '2 (copy)'\n");
    if (N < 2) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 2);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed '2 (copy) (copy)'\n"
                                     "destroyed '1 (copy) (copy)'\n"
                                     "destroyed 'allocator (copy) (copy)'\n"
                                     "destroyed '2 (copy)'\n"
                                     "destroyed '1 (copy)'\n"
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
        SMFTracker("1"),
        SMFTracker("2"),
        SMFTracker("3"),
    };
    EXPECT(other.len() == 3, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == max(3UZ, N), "capacity = %zu\n",
           other.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed '1'\n"
                                       "constructed '2'\n"
                                       "constructed '3'\n"
                                       "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "copy constructed '1'\n"
                                       "copy constructed '2'\n"
                                       "copy constructed '3'\n"
                                       "destroyed 'allocator'\n"
                                       "destroyed '3'\n"
                                       "destroyed '2'\n"
                                       "destroyed '1'\n");
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
                                         "move constructed '1 (copy)'\n"
                                         "move constructed '2 (copy)'\n"
                                         "move constructed '3 (copy)'\n"
                                         "destroyed '3 (copy)'\n"
                                         "destroyed '2 (copy)'\n"
                                         "destroyed '1 (copy)'\n");
    }
    msg[0] = '\0';
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  if (N < 3) {
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed '3 (copy)'\n"
                                       "destroyed '2 (copy)'\n"
                                       "destroyed '1 (copy)'\n"
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
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed '3 (copy) (moved)'\n"
                                       "destroyed '2 (copy) (moved)'\n"
                                       "destroyed '1 (copy) (moved)'\n"
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
        SMFTracker("1"),
        SMFTracker("2"),
    };
    TestAllocator alloc;

    EXPECT(other.len() == 2, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == max(2UZ, N), "capacity = %zu\n",
           other.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed '1'\n"
                                       "constructed '2'\n"
                                       "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "copy constructed '1'\n"
                                       "copy constructed '2'\n"
                                       "destroyed 'allocator'\n"
                                       "destroyed '2'\n"
                                       "destroyed '1'\n"
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
                                       "copy constructed '1 (copy)'\n"
                                       "copy constructed '2 (copy)'\n");
    if (N < 2) {
      snprintf(msg.data(), msg.len(), "allocated %zu bytes\n",
               sizeof(SMFTracker) * 2);
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed '2 (copy) (copy)'\n"
                                     "destroyed '1 (copy) (copy)'\n"
                                     "destroyed 'allocator (copy)'\n"
                                     "destroyed 'allocator'\n"
                                     "destroyed '2 (copy)'\n"
                                     "destroyed '1 (copy)'\n"
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
        SMFTracker("1"),
        SMFTracker("2"),
        SMFTracker("3"),
    };
    TestAllocator alloc;

    EXPECT(other.len() == 3, "len = %zu.\n", other.len());
    EXPECT(other.capacity() == max(3UZ, N), "capacity = %zu\n",
           other.capacity());
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "constructed '1'\n"
                                       "constructed '2'\n"
                                       "constructed '3'\n"
                                       "constructed 'allocator'\n"
                                       "copy constructed 'allocator'\n"
                                       "copy constructed '1'\n"
                                       "copy constructed '2'\n"
                                       "copy constructed '3'\n"
                                       "destroyed 'allocator'\n"
                                       "destroyed '3'\n"
                                       "destroyed '2'\n"
                                       "destroyed '1'\n"
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
                                         "move constructed '1 (copy)'\n"
                                         "move constructed '2 (copy)'\n"
                                         "move constructed '3 (copy)'\n"
                                         "destroyed '3 (copy)'\n"
                                         "destroyed '2 (copy)'\n"
                                         "destroyed '1 (copy)'\n");
    }
    msg[0] = '\0';
    EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());

    clear_logs();
  }

  if (N < 3) {
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed '3 (copy)'\n"
                                       "destroyed '2 (copy)'\n"
                                       "destroyed '1 (copy)'\n"
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
    EXPECT_STREQ(SMFTracker::LOGS.ptr, "destroyed '3 (copy) (moved)'\n"
                                       "destroyed '2 (copy) (moved)'\n"
                                       "destroyed '1 (copy) (moved)'\n"
                                       "destroyed 'allocator (copy)'\n"
                                       "destroyed 'allocator'\n"
                                       "destroyed 'allocator (copy)'\n");
  }
  EXPECT_STREQ(TestAllocator::LOGS.ptr, msg.data());
}

template <yacsl::usize N>
static void test_copy_assignment(yacsl::u32 len_a, yacsl::u32 len_b,
                                 bool allocs_eq) {
  TestAllocator::ALWAYS_RETURN_EQUAL = allocs_eq;
  {
    using yacsl::containers::StackArray;

    auto vec_a = Vector<SMFTracker, N, TestAllocator>(len_a);
    auto vec_b = Vector<SMFTracker, N, TestAllocator>(len_b);

    for (yacsl::u32 i = 0; i < len_a; ++i) {
      vec_a[i].name_.clear();
      vec_a[i].name_.printf_to("A%u", i);
    }

    for (yacsl::u32 i = 0; i < len_b; ++i) {
      vec_b[i].name_.clear();
      vec_b[i].name_.printf_to("B%u", i);
    }

    clear_logs();

    vec_a = vec_b;

    EXPECT(vec_a.len() == vec_b.len());
    EXPECT(vec_a.data() != vec_b.data());

    StackArray<char, 256> logs    = {};
    size_t                printed = 0;
    for (size_t i = len_a; i > 0; --i) {
      printed += static_cast<size_t>(snprintf(logs.data() + printed,
                                              logs.len() - printed,
                                              "destroyed 'A%zu'\n", i - 1));
      if (printed > logs.len()) {
        fputs("logs is too small\n", stderr);
        return;
      }
    }
    printed += static_cast<size_t>(
        snprintf(logs.data() + printed, logs.len() - printed,
                 "copy assigned 'allocator (copy)'\n"));
    if (printed > logs.len()) {
      FAIL("logs is too small\n");
      return;
    }
    for (size_t i = 0; i < len_b; ++i) {
      printed += static_cast<size_t>(snprintf(logs.data() + printed,
                                              logs.len() - printed,
                                              "copy constructed 'B%zu'\n", i));
      if (printed > logs.len()) {
        FAIL("logs is too small\n");
        return;
      }
    }
    EXPECT_STREQ(SMFTracker::LOGS.ptr, logs.data());

    logs[0] = '\0';
    printed = 0;
    if (allocs_eq && len_b > len_a) {
      if (N == 0 || len_b > N) {
        printed += static_cast<size_t>(snprintf(logs.data(), logs.len(),
                                                "allocated %zu bytes\n",
                                                sizeof(SMFTracker) * len_b));
      }
      if (N == 0 || len_a > N) {
        snprintf(logs.data() + printed, logs.len() - printed,
                 "deallocated %zu bytes\n", sizeof(SMFTracker) * len_a);
      }
    } else if (!allocs_eq) {
      if (N == 0 || len_a > N) {
        printed += static_cast<size_t>(snprintf(logs.data(), logs.len(),
                                                "deallocated %zu bytes\n",
                                                sizeof(SMFTracker) * len_a));
      }
      if (N == 0 || len_b > N) {
        snprintf(logs.data() + printed, logs.len() - printed,
                 "allocated %zu bytes\n", sizeof(SMFTracker) * len_b);
      }
    }
    EXPECT_STREQ(TestAllocator::LOGS.ptr, logs.data());
  }
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

#define TEST_COPY_ASSIGNMENT(N, len_a, len_b, allocs_eq, postfix)              \
  run_test("Vector<" #N ">::operator=(other) with a.len = " #len_a             \
           ", b.len = " #len_b ", " postfix,                                   \
           test_copy_assignment<N>, len_a, len_b, allocs_eq)

#define TEST_COPY_ASSIGNMENT8(len_a, len_b, allocs_eq, postfix)                \
  TEST_COPY_ASSIGNMENT(1, len_a, len_b, allocs_eq, postfix);                   \
  TEST_COPY_ASSIGNMENT(2, len_a, len_b, allocs_eq, postfix);                   \
  TEST_COPY_ASSIGNMENT(3, len_a, len_b, allocs_eq, postfix);                   \
  TEST_COPY_ASSIGNMENT(4, len_a, len_b, allocs_eq, postfix);                   \
  TEST_COPY_ASSIGNMENT(5, len_a, len_b, allocs_eq, postfix);                   \
  TEST_COPY_ASSIGNMENT(6, len_a, len_b, allocs_eq, postfix);                   \
  TEST_COPY_ASSIGNMENT(7, len_a, len_b, allocs_eq, postfix);                   \
  TEST_COPY_ASSIGNMENT(8, len_a, len_b, allocs_eq, postfix)

#define TEST_COPY_ASSIGNMENT32(len_b, allocs_eq, postfix)                      \
  TEST_COPY_ASSIGNMENT8(0U, len_b, allocs_eq, postfix);                        \
  TEST_COPY_ASSIGNMENT8(1U, len_b, allocs_eq, postfix);                        \
  TEST_COPY_ASSIGNMENT8(2U, len_b, allocs_eq, postfix);                        \
  TEST_COPY_ASSIGNMENT8(3U, len_b, allocs_eq, postfix)

#define TEST_COPY_ASSIGNMENT128(allocs_eq, postfix)                            \
  TEST_COPY_ASSIGNMENT32(0U, allocs_eq, postfix);                              \
  TEST_COPY_ASSIGNMENT32(1U, allocs_eq, postfix);                              \
  TEST_COPY_ASSIGNMENT32(2U, allocs_eq, postfix);                              \
  TEST_COPY_ASSIGNMENT32(3U, allocs_eq, postfix)

  TEST_COPY_ASSIGNMENT128(false, "a.alloc != b.alloc");
  TEST_COPY_ASSIGNMENT128(true, "a.alloc == b.alloc");

  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
