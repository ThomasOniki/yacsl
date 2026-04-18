#include "base/containers/vector.hpp"
#include "base/threads/atomic.hpp"
#include "base/threads/mutex.hpp"
#include "base/threads/thread.hpp"
#include "test_utils.hpp"
#include <stdint.h>

static int test_atomic(uint32_t thread_count) {
	int												 ret = 0;
	base::Vector<base::Thread> threads;
	base::Atomic<uint32_t>		 count = 0;

	{
		EXPECT_EQ(count.load(base::MemoryOrder::Relaxed), 0);

		for (uint32_t i = 0; i < thread_count; ++i) {
			threads.emplace_back([](base::Atomic<uint32_t>& lambda_count) { ++lambda_count; }, count);
		}
	}

	for (uint32_t i = 0; i < thread_count; ++i) {
		threads[i].join();
	}

	EXPECT_EQ(count.load(base::MemoryOrder::Relaxed), thread_count);
	return ret;
}

static int test_mutex(uint32_t thread_count) {
	int												 ret = 0;
	base::Vector<base::Thread> threads;
	uint32_t									 count = 0;

	base::Mutex mutex;

	{
		EXPECT_EQ(count, 0);

		for (uint32_t i = 0; i < thread_count; ++i) {
			threads.emplace_back(
					[&mutex](uint32_t& lambda_count) {
						base::MutexLock lock(mutex);
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
	EXPECT_EQ(test_atomic(1), 0);
	EXPECT_EQ(test_atomic(1000), 0);
	EXPECT_EQ(test_mutex(1), 0);
	EXPECT_EQ(test_mutex(1000), 0);
	return ret;
}
