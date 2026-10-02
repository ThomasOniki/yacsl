#include "test_utils.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/debug/log.hpp"
#include "yacsl/threads/atomic.hpp"
#include "yacsl/threads/mutex.hpp"
#include "yacsl/threads/thread.hpp"
#include <stdint.h>

static int test_atomic(uint32_t thread_count) {
  int                                               ret = 0;
  yacsl::containers::Vector<yacsl::threads::Thread> threads;
  yacsl::threads::Atomic<uint32_t>                  count = 0;

  {
    EXPECT_EQ(count.load(yacsl::threads::MemoryOrder::Relaxed), 0);

    for (uint32_t i = 0; i < thread_count; ++i) {
      threads.emplace_back(
          [](yacsl::threads::Atomic<uint32_t>& lambda_count) {
            ++lambda_count;
          },
          count);
    }
  }

  for (uint32_t i = 0; i < thread_count; ++i) {
    threads[i].join();
  }

  EXPECT_EQ(count.load(yacsl::threads::MemoryOrder::Relaxed), thread_count);
  return ret;
}

static int test_mutex(uint32_t thread_count) {
  int                                               ret = 0;
  yacsl::containers::Vector<yacsl::threads::Thread> threads;
  uint32_t                                          count = 0;

  yacsl::threads::Mutex mutex;

  {
    EXPECT_EQ(count, 0);

    for (uint32_t i = 0; i < thread_count; ++i) {
      threads.emplace_back(
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

  EXPECT_EQ(count, thread_count);
  return ret;
}

int main() {
  int ret = 0;
  EXPECT(test_atomic(1) == 0);
  EXPECT(test_atomic(100) == 0);
  EXPECT(test_mutex(1) == 0);
  EXPECT(test_mutex(100) == 0);
  yacsl::debug::Logger::get_default_instance().flush();
  return ret;
}
