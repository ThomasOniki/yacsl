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

template <typename T, usize N> class MPSCRingBuffer {
public:
  MPSCRingBuffer() noexcept {
    for (usize i = 0; i < N; ++i) {
      slots_[i].epoch.store(i, MemoryOrder::SeqCst);
    }
  }

  ~MPSCRingBuffer() noexcept = default;

  MPSCRingBuffer(MPSCRingBuffer const&)            = delete;
  MPSCRingBuffer(MPSCRingBuffer&&)                 = delete;
  MPSCRingBuffer& operator=(MPSCRingBuffer const&) = delete;
  MPSCRingBuffer& operator=(MPSCRingBuffer&&)      = delete;

  template <typename... Args>
  T& emplace(this MPSCRingBuffer& self, Args&&... args) {
    usize const write = self.write_.fetch_add(1, MemoryOrder::Relaxed);
    usize const index = write % N;
    while (self.slots_[index].epoch.load(MemoryOrder::Acquire) != write) {
      sched_yield();
    }
    self.slots_[index].value.construct(base::forward<Args>(args)...);
    self.slots_[index].epoch.store(write + 1, MemoryOrder::Release);
    return *self.slots_[index].value;
  }

  T& push(this MPSCRingBuffer& self, T&& value) {
    return self.emplace(base::move(value));
  }
  T& push(this MPSCRingBuffer& self, T const& value) {
    return self.emplace(value);
  }

  T pop(this MPSCRingBuffer& self) noexcept {
    usize const index = self.read_ % N;
    while (self.slots_[index].epoch.load(MemoryOrder::Acquire) !=
           self.read_ + 1) {
      sched_yield();
    }
    T ret(base::move(*self.slots_[index].value));
    self.slots_[index].value.destruct();
    self.slots_[index].epoch.store(self.read_ + N, MemoryOrder::Release);
    ++self.read_;
    return ret;
  }

  template <typename... Args>
  bool try_emplace(this MPSCRingBuffer& self, Args&&... args) {
    for (;;) {
      usize       write = self.write_.load(MemoryOrder::Relaxed);
      usize const index = write % N;
      usize const diff =
          self.slots_[index].epoch.load(MemoryOrder::Acquire) - write;
      if (diff == 0) {
        if (self.write_.compare_exchange_weak(
                write, write + 1, MemoryOrder::Relaxed, MemoryOrder::Relaxed)) {
          self.slots_[index].value.construct(base::forward<Args>(args)...);
          self.slots_[index].epoch.store(write + 1, MemoryOrder::Release);
          return true;
        }
        // If CAS fails, another producer wrote it between self.write_.load and
        // self.write_.CAS so we retry.
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
      // self.write_.load and epoch.load so we retry.
    }
  }

  bool try_push(this MPSCRingBuffer& self, T&& value) {
    return self.try_emplace(base::move(value));
  }
  bool try_push(this MPSCRingBuffer& self, T const& value) {
    return self.try_emplace(value);
  }

  Option<T> try_pop(this MPSCRingBuffer& self) noexcept {
    usize const index = self.read_ % N;
    if (self.slots_[index].epoch.load(MemoryOrder::Acquire) != self.read_ + 1) {
      return NIL;
    }
    T ret(*self.slots_[index].value);
    self.slots_[index].value.destruct();
    self.slots_[index].epoch.store(self.read_ + N, MemoryOrder::Release);
    ++self.read_;
    return ret;
  }

private:
  struct Slot {
    StackStorage<T> value;
    Atomic<usize>   epoch;
  };
  StackArray<Slot, N, usize> slots_;
  alignas(128) Atomic<usize> write_ = 0;
  alignas(128) usize read_          = 0;
};

} // namespace base
