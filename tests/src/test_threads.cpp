#include "test_utils.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/testing/run.hpp"
#include "yacsl/threads/atomic.hpp"
#include "yacsl/threads/mutex.hpp"
#include "yacsl/threads/thread.hpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static void test_atomic(uint32_t thread_count) {
  yacsl::containers::Vector<yacsl::threads::Thread> threads;
  yacsl::threads::Atomic<uint32_t>                  count = 0;

  {
    uint32_t const loaded = count.load(yacsl::threads::MemoryOrder::Relaxed);
    EXPECT(loaded == 0);

    for (uint32_t i = 0; i < thread_count; ++i) {
      threads.push(
          [](yacsl::threads::Atomic<uint32_t>& lambda_count) {
            ++lambda_count;
          },
          count);
    }
  }

  for (uint32_t i = 0; i < thread_count; ++i) {
    threads[i].join();
  }

  uint32_t const loaded = count.load(yacsl::threads::MemoryOrder::Relaxed);
  EXPECT(loaded == thread_count);
}

static void test_mutex(uint32_t thread_count) {
  yacsl::containers::Vector<yacsl::threads::Thread> threads;
  uint32_t                                          count = 0;

  yacsl::threads::Mutex mutex;

  {
    for (uint32_t i = 0; i < thread_count; ++i) {
      threads.push(
          [&mutex](uint32_t& lambda_count) {
            yacsl::threads::MutexLock const lock(mutex);
            ++lambda_count;
          },
          count);
    }
  }

  for (uint32_t i = 0; i < thread_count; ++i) {
    threads[i].join();
  }

  EXPECT(count == thread_count);
}

int main() {
  run_test("Atomic (1 thread)", test_atomic, 1U);
  run_test("Atomic (100 threads)", test_atomic, 100U);
  run_test("Mutex (1 thread)", test_mutex, 1U);
  run_test("Mutex (100 threads)", test_mutex, 100U);
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
