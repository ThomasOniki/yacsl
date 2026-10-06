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
static void test(yacsl::testing::Test& test) noexcept {
  yacsl::threads::Atomic<bool>                      start = false;
  yacsl::containers::Vector<yacsl::threads::Thread> producers;
  Ring                                              ring;

  for (yacsl::usize i = 0; i < SLOTS; ++i) {
    if (!ring.try_push(i)) {
      test.fail_quietly();
      fprintf(stderr, "Failed to try_push %zu to ring buffer.\n", i);
    }
  }
  if (ring.try_push(0)) {
    test.fail_quietly();
    fputs("try_push() succeeded on a full ring buffer.\n", stderr);
  }
  for (yacsl::usize i = 0; i < SLOTS; ++i) {
    auto opt = ring.try_pop();
    if (opt != i) {
      test.fail_quietly();
      if (opt.is_nil()) {
        fputs("Failed to pop from ring buffer.\n", stderr);
      }
      fprintf(stderr, "The value poped from ring buffer was %zu, not %zu.\n",
              opt.value_unchecked(), i);
    }
  }
  if (ring.try_pop().is_value()) {
    test.fail_quietly();
    fputs("try_pop() succeeded on an empty ring buffer.\n", stderr);
  }
  for (yacsl::usize i = 0; i < Producers; ++i) {
    producers.emplace_back([&ring, &start, i] {
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
    consumers.emplace_back([&ring, &start, &did_pop, &pop_count] {
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
    if (!did_pop[index]) {
      test.fail_quietly();
      fprintf(stderr, "%zuth value was not poped/consumed\n", index);
    }
  }
}

int main() {
  int                           ret   = 0;
  yacsl::containers::StackArray tests = {
      yacsl::testing::Test(
          "MPSC",
          test<yacsl::threads::MPSCRingBuffer<yacsl::usize, SLOTS>, 50, 1>),
      yacsl::testing::Test(
          "MPMC",
          test<yacsl::threads::MPMCRingBuffer<yacsl::usize, SLOTS>, 50, 50>),
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
