#include "yacsl/containers/stack_array.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/testing/test.hpp"
#include "yacsl/threads/atomic.hpp"
#include "yacsl/threads/mutex.hpp"
#include "yacsl/threads/thread.hpp"
#include <cstdint>
#include <stdint.h>

template <uint32_t ThreadCount>
static void test_atomic(yacsl::testing::Test& test) {
  yacsl::containers::Vector<yacsl::threads::Thread> threads;
  yacsl::threads::Atomic<uint32_t>                  count = 0;

  {
    uint32_t loaded = count.load(yacsl::threads::MemoryOrder::Relaxed);
    if (loaded != 0) {
      test.fail_quietly();
      fprintf(stderr, "Loaded %u 0 from zero initialized atomic.\n", loaded);
    }

    for (uint32_t i = 0; i < ThreadCount; ++i) {
      threads.emplace_back(
          [](yacsl::threads::Atomic<uint32_t>& lambda_count) {
            ++lambda_count;
          },
          count);
    }
  }

  for (uint32_t i = 0; i < ThreadCount; ++i) {
    threads[i].join();
  }

  uint32_t loaded = count.load(yacsl::threads::MemoryOrder::Relaxed);
  if (loaded != ThreadCount) {
    test.fail_quietly();
    fprintf(stderr, "Atomic counter contains %u, not %u.\n", loaded,
            ThreadCount);
  }
}

template <uint32_t ThreadCount>
static void test_mutex(yacsl::testing::Test& test) {
  yacsl::containers::Vector<yacsl::threads::Thread> threads;
  uint32_t                                          count = 0;

  yacsl::threads::Mutex mutex;

  {
    for (uint32_t i = 0; i < ThreadCount; ++i) {
      threads.emplace_back(
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

  if (count != ThreadCount) {
    test.fail_quietly();
    fprintf(stderr, "Mutex protected counter contains %u, not %u.\n", count,
            ThreadCount);
  }
}

int main() {
  int                           ret   = 0;
  yacsl::containers::StackArray tests = {
      yacsl::testing::Test("Atomic (1 thread)", test_atomic<1>),
      yacsl::testing::Test("Atomic (100 threads)", test_atomic<100>),
      yacsl::testing::Test("Mutex (1 thread)", test_mutex<1>),
      yacsl::testing::Test("Mutex (100 threads)", test_mutex<100>),
  };
  for (auto& test : tests) {
    printf("Running test '%s'...\n", test.cstr_name());
    if (test.run_quietly() == yacsl::testing::Test::Result::Failure) {
      fprintf(stderr, "test '%s' failed.\n", test.cstr_name());
      ret = 1;
    }
  }
  return ret;
}
