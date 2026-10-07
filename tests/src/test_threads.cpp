#include "test_utils.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/testing/test.hpp"
#include "yacsl/threads/atomic.hpp"
#include "yacsl/threads/mutex.hpp"
#include "yacsl/threads/thread.hpp"
#include <stdint.h>
#include <stdio.h>

template <uint32_t ThreadCount>
static void test_atomic(yacsl::testing::Test& /*test*/) {
  yacsl::containers::Vector<yacsl::threads::Thread> threads;
  yacsl::threads::Atomic<uint32_t>                  count = 0;

  {
    uint32_t const loaded = count.load(yacsl::threads::MemoryOrder::Relaxed);
    EXPECT(loaded == 0);

    for (uint32_t i = 0; i < ThreadCount; ++i) {
      threads.push(
          [](yacsl::threads::Atomic<uint32_t>& lambda_count) {
            ++lambda_count;
          },
          count);
    }
  }

  for (uint32_t i = 0; i < ThreadCount; ++i) {
    threads[i].join();
  }

  uint32_t const loaded = count.load(yacsl::threads::MemoryOrder::Relaxed);
  EXPECT(loaded == ThreadCount);
}

template <uint32_t ThreadCount>
static void test_mutex(yacsl::testing::Test& /*test*/) {
  yacsl::containers::Vector<yacsl::threads::Thread> threads;
  uint32_t                                          count = 0;

  yacsl::threads::Mutex mutex;

  {
    for (uint32_t i = 0; i < ThreadCount; ++i) {
      threads.push(
          [&mutex](uint32_t& lambda_count) {
            yacsl::threads::MutexLock const lock(mutex);
            ++lambda_count;
          },
          count);
    }
  }

  for (uint32_t i = 0; i < ThreadCount; ++i) {
    threads[i].join();
  }

  EXPECT(count == ThreadCount);
}

int main() {
  yacsl::containers::StackArray tests = {
      yacsl::testing::Test("Atomic (1 thread)", test_atomic<1>),
      yacsl::testing::Test("Atomic (100 threads)", test_atomic<100>),
      yacsl::testing::Test("Mutex (1 thread)", test_mutex<1>),
      yacsl::testing::Test("Mutex (100 threads)", test_mutex<100>),
  };
  return run_tests(tests.slice());
}
