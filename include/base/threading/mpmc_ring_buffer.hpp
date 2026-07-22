#pragma once

#include "base/containers/option.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/stack_storage.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/threading/atomic.hpp"
#include "base/type_traits/limits.hpp"
#include "base/utilities/nil.hpp"
#include "base/utilities/type_aliases.hpp"
#include <sched.h>

namespace base {

template <typename T, usize N> class MPMCRingBuffer {
public:
  MPMCRingBuffer() noexcept {
    for (usize i = 0; i < N; ++i) {
      slots_[i].epoch.store(i, MemoryOrder::SeqCst);
    }
  }

  ~MPMCRingBuffer() noexcept = default;

  MPMCRingBuffer(const MPMCRingBuffer&)            = delete;
  MPMCRingBuffer(MPMCRingBuffer&&)                 = delete;
  MPMCRingBuffer& operator=(const MPMCRingBuffer&) = delete;
  MPMCRingBuffer& operator=(MPMCRingBuffer&&)      = delete;

  template <typename... Args> void emplace(Args&&... args) {
    usize write = write_.fetch_add(1, MemoryOrder::Relaxed);
    usize index = write % N;
    while (slots_[index].epoch.load(MemoryOrder::Acquire) != write) {
      sched_yield();
    }
    slots_[index].value.construct(base::forward<Args>(args)...);
    slots_[index].epoch.store(write + 1, MemoryOrder::Release);
  }

  void push(T&& value) { emplace(base::move(value)); }
  void push(const T& value) { emplace(value); }

  T pop() noexcept {
    usize read  = read_.fetch_add(1, MemoryOrder::Relaxed);
    usize index = read % N;
    while (slots_[index].epoch.load(MemoryOrder::Acquire) != read + 1) {
      sched_yield();
    }
    T ret(base::move(*slots_[index].value));
    slots_[index].value.destruct();
    slots_[index].epoch.store(read + N, MemoryOrder::Release);
    return ret;
  }

  template <typename... Args> bool try_emplace(Args&&... args) {
    for (;;) {
      usize write = write_.load(MemoryOrder::Relaxed);
      usize index = write % N;
      usize diff  = slots_[index].epoch.load(MemoryOrder::Acquire) - write;
      if (diff == 0) {
        if (write_.compare_exchange_weak(write, write + 1, MemoryOrder::Relaxed,
                                         MemoryOrder::Relaxed)) {
          slots_[index].value.construct(base::forward<Args>(args)...);
          slots_[index].epoch.store(write + 1, MemoryOrder::Release);
          return true;
        }
        // If CAS fails, another producer wrote it between write_.load and write_.CAS so we retry.
        continue;
      }
      // Since usize has a finite precision and wraps to 0 on overflow and max on underflow, there
      // is no way to truly know if the write pointer is ahead of the epoch. This assumes that there
      // are less than Limits<usize>::max/2 threads. If the assumption holds and diff > max/2, then
      // the true signed infinite precision difference must be negative and have wrapped around to
      // become a big number. Otherwise it must be positive. If the true difference is negative,
      // epoch < write. This means that the slot hasn't been poped yet; the queue is full.
      if (diff > Limits<usize>::max / 2) {
        return false;
      }
      // If epoch > write, another producer changed the epoch between write_.load and epoch.load so
      // we retry.
    }
  }

  bool try_push(T&& value) { return try_emplace(base::move(value)); }
  bool try_push(const T& value) { return try_emplace(value); }

  Option<T> try_pop() noexcept {
    for (;;) {
      usize read  = read_.load(MemoryOrder::Relaxed);
      usize index = read % N;
      usize diff  = slots_[index].epoch.load(MemoryOrder::Acquire) - (read + 1);
      if (diff == 0) {
        if (read_.compare_exchange_weak(read, read + 1, MemoryOrder::Relaxed,
                                        MemoryOrder::Relaxed)) {
          T ret(*slots_[index].value);
          slots_[index].value.destruct();
          slots_[index].epoch.store(read + N, MemoryOrder::Release);
          return ret;
        }
        continue;
      }
      if (diff > Limits<usize>::max / 2) {
        return NIL;
      }
    }
  }

private:
  struct Slot {
    StackStorage<T> value;
    Atomic<usize>   epoch;
  };
  StackArray<Slot, N, usize> slots_;
  alignas(128) Atomic<usize> write_ = 0;
  alignas(128) Atomic<usize> read_  = 0;
};

} // namespace base
