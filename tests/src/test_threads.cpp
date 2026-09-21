#include "base/containers/vector.hpp"
#include "base/debug/log.hpp"
#include "base/threads/atomic.hpp"
#include "base/threads/mutex.hpp"
#include "base/threads/thread.hpp"
#include "test_utils.hpp"
#include <stdint.h>

static int test_atomic(uint32_t thread_count) {
  int                                             ret = 0;
  base::containers::Vector<base::threads::Thread> threads;
  base::threads::Atomic<uint32_t>                 count = 0;

  {
    EXPECT_EQ(count.load(base::threads::MemoryOrder::Relaxed), 0);

    for (uint32_t i = 0; i < thread_count; ++i) {
      threads.emplace_back(
          [](base::threads::Atomic<uint32_t>& lambda_count) { ++lambda_count; },
          count);
    }
  }

  for (uint32_t i = 0; i < thread_count; ++i) {
    threads[i].join();
  }

  EXPECT_EQ(count.load(base::threads::MemoryOrder::Relaxed), thread_count);
  return ret;
}

static int test_mutex(uint32_t thread_count) {
  int                                             ret = 0;
  base::containers::Vector<base::threads::Thread> threads;
  uint32_t                                        count = 0;

  base::threads::Mutex mutex;

  {
    EXPECT_EQ(count, 0);

    for (uint32_t i = 0; i < thread_count; ++i) {
      threads.emplace_back(
          [&mutex](uint32_t& lambda_count) {
            base::threads::MutexLock const lock(mutex);
            ++lambda_count;
          },
          count);
    }
  }

  for (uint32_t i = 0; i < thread_count; ++i) {
    threads[i].join();
  }

  EXPECT_EQ(count, thread_count);
  return ret;
}

int main() {
  int ret = 0;
  EXPECT(test_atomic(1) == 0);
  EXPECT(test_atomic(100) == 0);
  EXPECT(test_mutex(1) == 0);
  EXPECT(test_mutex(100) == 0);
  base::debug::Logger::get_default_instance().flush();
  return ret;
}
