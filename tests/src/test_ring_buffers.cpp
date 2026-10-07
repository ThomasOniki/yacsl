#include "test_utils.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/testing/test.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/yacsl.hpp"
#include <stdio.h>

static constexpr yacsl::usize SLOTS = 64;

#ifdef YACSL_LSP_ONLY
using Ring = yacsl::threads::MPSCRingBuffer<yacsl::usize, SLOTS>;
template <typename _, yacsl::usize Producers, yacsl::usize Consumers>
#else
template <typename Ring, yacsl::usize Producers, yacsl::usize Consumers>
#endif
static void test(yacsl::testing::Test& /*test*/) noexcept {
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
  for (yacsl::usize i = 0; i < Producers; ++i) {
    producers.push([&ring, &start, i] {
      while (!start.load(yacsl::threads::MemoryOrder::Acquire)) {
      }
      ring.push(i);
    });
  }

  yacsl::containers::Vector<yacsl::threads::Thread> consumers;

  yacsl::containers::Vector<bool> did_pop;
  did_pop.resize(Producers);

  yacsl::threads::Atomic<yacsl::usize> pop_count;

  for (yacsl::usize i = 0; i < Consumers; ++i) {
    consumers.push([&ring, &start, &did_pop, &pop_count] {
      while (!start.load(yacsl::threads::MemoryOrder::Acquire)) {
      }
      while (pop_count.fetch_add(1, yacsl::threads::MemoryOrder::Relaxed) <
             Producers) {
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
  yacsl::containers::StackArray tests = {
      yacsl::testing::Test(
          "MPSC",
          test<yacsl::threads::MPSCRingBuffer<yacsl::usize, SLOTS>, 50, 1>),
      yacsl::testing::Test(
          "MPMC",
          test<yacsl::threads::MPMCRingBuffer<yacsl::usize, SLOTS>, 50, 50>),
  };
  return run_tests(tests.slice());
}
