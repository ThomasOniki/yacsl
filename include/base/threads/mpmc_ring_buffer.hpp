#pragma once

#include "base/containers/option.hpp"
#include "base/containers/stack_array.hpp"
#include "base/containers/stack_storage.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/threads/atomic.hpp"
#include "base/type_aliases.hpp"
#include "base/type_traits/limits.hpp"
#include "base/utils/nil.hpp"
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

  MPMCRingBuffer(MPMCRingBuffer const&)            = delete;
  MPMCRingBuffer(MPMCRingBuffer&&)                 = delete;
  MPMCRingBuffer& operator=(MPMCRingBuffer const&) = delete;
  MPMCRingBuffer& operator=(MPMCRingBuffer&&)      = delete;

  template <typename... Args>
  T& emplace(this MPMCRingBuffer& self, Args&&... args) {
    usize write = self.write_.fetch_add(1, MemoryOrder::Relaxed);
    usize index = write % N;
    while (self.slots_[index].epoch.load(MemoryOrder::Acquire) != write) {
      sched_yield();
    }
    self.slots_[index].value.construct(base::forward<Args>(args)...);
    self.slots_[index].epoch.store(write + 1, MemoryOrder::Release);
    return *self.slots_[index].value;
  }

  T& push(this MPMCRingBuffer& self, T&& value) {
    return self.emplace(base::move(value));
  }
  T& push(this MPMCRingBuffer& self, T const& value) {
    return self.emplace(value);
  }

  T pop(this MPMCRingBuffer& self) noexcept {
    usize read  = self.read_.fetch_add(1, MemoryOrder::Relaxed);
    usize index = read % N;
    while (self.slots_[index].epoch.load(MemoryOrder::Acquire) != read + 1) {
      sched_yield();
    }
    T ret(base::move(*self.slots_[index].value));
    self.slots_[index].value.destruct();
    self.slots_[index].epoch.store(read + N, MemoryOrder::Release);
    return ret;
  }

  template <typename... Args>
  bool try_emplace(this MPMCRingBuffer& self, Args&&... args) {
    for (;;) {
      usize write = self.write_.load(MemoryOrder::Relaxed);
      usize index = write % N;
      usize diff  = self.slots_[index].epoch.load(MemoryOrder::Acquire) - write;
      if (diff == 0) {
        if (self.write_.compare_exchange_weak(
                write, write + 1, MemoryOrder::Relaxed, MemoryOrder::Relaxed)) {
          self.slots_[index].value.construct(base::forward<Args>(args)...);
          self.slots_[index].epoch.store(write + 1, MemoryOrder::Release);
          return true;
        }
        // If CAS fails, another producer wrote it between write_.load and
        // write_.CAS so we retry.
        continue;
      }
      // Since usize has a finite precision and wraps to 0 on overflow and max
      // on underflow, there is no way to truly know if the write pointer is
      // ahead of the epoch. This assumes that there are less than
      // Limits<usize>::MAX/2 threads. If the assumption holds and diff > max/2,
      // then the true signed infinite precision difference must be negative and
      // have wrapped around to become a big number. Otherwise it must be
      // positive. If the true difference is negative, epoch < write. This means
      // that the slot hasn't been poped yet; the queue is full.
      if (diff > Limits<usize>::MAX / 2) {
        return false;
      }
      // If epoch > write, another producer changed the epoch between
      // write_.load and epoch.load so we retry.
    }
  }

  bool try_push(this MPMCRingBuffer& self, T&& value) {
    return self.try_emplace(base::move(value));
  }
  bool try_push(this MPMCRingBuffer& self, T const& value) {
    return self.try_emplace(value);
  }

  Option<T> try_pop(this MPMCRingBuffer& self) noexcept {
    for (;;) {
      usize       read  = self.read_.load(MemoryOrder::Relaxed);
      usize const index = read % N;
      usize const diff =
          self.slots_[index].epoch.load(MemoryOrder::Acquire) - (read + 1);
      if (diff == 0) {
        if (self.read_.compare_exchange_weak(
                read, read + 1, MemoryOrder::Relaxed, MemoryOrder::Relaxed)) {
          T ret(*self.slots_[index].value);
          self.slots_[index].value.destruct();
          self.slots_[index].epoch.store(read + N, MemoryOrder::Release);
          return ret;
        }
        continue;
      }
      if (diff > Limits<usize>::MAX / 2) {
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
