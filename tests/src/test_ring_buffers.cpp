#include "yacsl/yacsl.hpp"
#include "test_utils.hpp"

static constexpr yacsl::usize SLOTS = 64;

namespace {
struct Counts {
  yacsl::usize producers;
  yacsl::usize consumers;
};
} // namespace

#ifdef YACSL_LSP_ONLY
using Ring = yacsl::threads::MPSCRingBuffer<yacsl::usize, SLOTS>;
template <typename _>
#else
template <typename Ring>
#endif
static int test(Counts counts) noexcept {
  int                                             ret   = 0;
  yacsl::threads::Atomic<bool>                     start = false;
  yacsl::containers::Vector<yacsl::threads::Thread> producers;
  Ring                                            ring;

  for (yacsl::usize i = 0; i < SLOTS; ++i) {
    EXPECT(ring.try_push(i));
  }
  EXPECT(!ring.try_push(0));
  for (yacsl::usize i = 0; i < SLOTS; ++i) {
    EXPECT(ring.try_pop() == i);
  }
  EXPECT(ring.try_pop().is_nil());

  for (yacsl::usize i = 0; i < counts.producers; ++i) {
    producers.emplace_back([&ring, &start, i]() {
      while (!start.load(yacsl::threads::MemoryOrder::Acquire)) {
      }
      ring.push(i);
    });
  }

  yacsl::containers::Vector<yacsl::threads::Thread> consumers;

  yacsl::containers::Vector<bool> did_pop;
  did_pop.resize(counts.producers);

  yacsl::threads::Atomic<yacsl::usize> pop_count;

  for (yacsl::usize i = 0; i < counts.consumers; ++i) {
    consumers.emplace_back([&ring, &start, &did_pop, &pop_count, counts]() {
      while (!start.load(yacsl::threads::MemoryOrder::Acquire)) {
      }
      while (pop_count.fetch_add(1, yacsl::threads::MemoryOrder::Relaxed) <
             counts.producers) {
        did_pop[ring.pop()] = true;
      }
    });
  }

  start.store(true, yacsl::threads::MemoryOrder::Release);

  for (yacsl::usize i = 0; i < counts.producers; ++i) {
    producers[i].join();
  }
  for (yacsl::usize i = 0; i < counts.consumers; ++i) {
    consumers[i].join();
  }

  for (bool const consumed : did_pop) {
    EXPECT(consumed);
  }

  return ret;
}

int main() {
  int ret = 0;
  EXPECT((test<yacsl::threads::MPSCRingBuffer<yacsl::usize, SLOTS>>(
              {.producers = 50, .consumers = 1}) == 0));
  EXPECT((test<yacsl::threads::MPMCRingBuffer<yacsl::usize, SLOTS>>(
              {.producers = 50, .consumers = 50}) == 0));
  return ret;
}
