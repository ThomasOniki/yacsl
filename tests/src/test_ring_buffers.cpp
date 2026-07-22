#include "base/base.hpp"
#include "test_utils.hpp"

static constexpr base::usize SLOTS = 64;

#ifdef BASE_LSP_ONLY
using Ring = base::MPSCRingBuffer<base::usize, SLOTS>;
template <typename _>
#else
template <typename Ring>
#endif
static int test(base::usize producer_count, base::usize consumer_count) noexcept {
  int                        ret   = 0;
  base::Atomic<bool>         start = false;
  base::Vector<base::Thread> producers;
  Ring                       ring;

  for (base::usize i = 0; i < SLOTS; ++i) {
    EXPECT(ring.try_push(i));
  }
  EXPECT(!ring.try_push(0));
  for (base::usize i = 0; i < SLOTS; ++i) {
    EXPECT(ring.try_pop() == i);
  }
  EXPECT(ring.try_pop().is_nil());

  for (base::usize i = 0; i < producer_count; ++i) {
    producers.emplace_back([&ring, &start, i]() {
      while (!start.load(base::MemoryOrder::Acquire)) {
      }
      ring.push(i);
    });
  }

  base::Vector<base::Thread> consumers;

  base::Vector<bool> did_pop;
  did_pop.resize(producer_count);

  base::Atomic<base::usize> pop_count;

  for (base::usize i = 0; i < consumer_count; ++i) {
    consumers.emplace_back([&ring, &start, &did_pop, &pop_count, producer_count]() {
      while (!start.load(base::MemoryOrder::Acquire)) {
      }
      while (pop_count.fetch_add(1, base::MemoryOrder::Relaxed) < producer_count) {
        did_pop[ring.pop()] = true;
      }
    });
  }

  start.store(true, base::MemoryOrder::Release);

  for (base::usize i = 0; i < producer_count; ++i) {
    producers[i].join();
  }
  for (base::usize i = 0; i < consumer_count; ++i) {
    consumers[i].join();
  }

  for (bool consumed : did_pop) {
    EXPECT(consumed);
  }

  return ret;
}

int main() {
  int ret = 0;
  EXPECT((test<base::MPSCRingBuffer<base::usize, SLOTS>>(50, 1) == 0));
  EXPECT((test<base::MPMCRingBuffer<base::usize, SLOTS>>(50, 50) == 0));
  return ret;
}
