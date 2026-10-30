#include "test_utils.hpp"
#include "yacsl/testing/run.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/yacsl.hpp"
#include <stdio.h>
#include <stdlib.h>

static constexpr yacsl::usize SLOTS = 64;

namespace {
struct TestOpts {
  yacsl::usize producer_count;
  yacsl::usize consumer_count;
};
} // namespace

#ifdef YACSL_LSP_ONLY
using Ring = yacsl::threads::MPSCRingBuffer<yacsl::usize, SLOTS>;
template <typename _>
#else
template <typename Ring>
#endif
static void test(TestOpts opts) noexcept {
  yacsl::threads::Atomic<bool>                      start = false;
  yacsl::containers::Vector<yacsl::threads::Thread> producers;
  Ring                                              ring;

  for (yacsl::usize i = 0; i < SLOTS; ++i) {
    EXPECT(ring.try_push(i));
  }
  EXPECT(!ring.try_push(0), "ring buffer should be full");
  for (yacsl::usize i = 0; i < SLOTS; ++i) {
    auto opt = ring.try_pop();
    if (opt != i) {
      if (opt.is_nil()) {
        FAIL("Failed to pop from ring buffer.\n");
      } else {
        FAIL("The value poped from ring buffer was %zu, not %zu.\n",
             opt.value_unchecked(), i);
      }
    }
  }
  EXPECT(ring.try_pop().is_nil(), "ring buffer should be empty");
  for (yacsl::usize i = 0; i < opts.producer_count; ++i) {
    producers.push([&ring, &start, i] {
      while (!start.load(yacsl::threads::MemoryOrder::Acquire)) {
      }
      ring.push(i);
    });
  }

  yacsl::containers::Vector<yacsl::threads::Thread> consumers;

  yacsl::containers::Vector<bool> did_pop;
  did_pop.resize(opts.producer_count);

  yacsl::threads::Atomic<yacsl::usize> pop_count;

  for (yacsl::usize i = 0; i < opts.consumer_count; ++i) {
    consumers.push([&] {
      while (!start.load(yacsl::threads::MemoryOrder::Acquire)) {
      }
      while (pop_count.fetch_add(1, yacsl::threads::MemoryOrder::Relaxed) <
             opts.producer_count) {
        did_pop[ring.pop()] = true;
      }
    });
  }

  start.store(true, yacsl::threads::MemoryOrder::Release);

  for (auto& producer : producers) {
    producer.join();
  }
  for (auto& consumer : consumers) {
    consumer.join();
  }

  for (yacsl::usize const index : did_pop.indices()) {
    EXPECT(did_pop[index]);
  }
}

int main() {
  run_test("MPSC", test<yacsl::threads::MPSCRingBuffer<yacsl::usize, SLOTS>>,
           TestOpts{.producer_count = 50UZ, .consumer_count = 1UZ});
  run_test("MPMC", test<yacsl::threads::MPMCRingBuffer<yacsl::usize, SLOTS>>,
           TestOpts{.producer_count = 50UZ, .consumer_count = 50UZ});
  return yacsl::testing::status() ? EXIT_SUCCESS : EXIT_FAILURE;
}
