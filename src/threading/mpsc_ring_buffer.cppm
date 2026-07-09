module;

#include <sched.h>

export module base.threading.mpsc_ring_buffer;

import base.utilities.type_aliases;
import base.threading.atomic;
import base.containers.stack_storage;
import base.containers.stack_array;
import base.containers.option;
import base.memory.lifetimes;
import base.type_traits.limits;
import base.utilities.nil;

export namespace base {

template <typename T, usize N> class MPSCRingBuffer {
public:
  MPSCRingBuffer() noexcept {
    for (usize i = 0; i < N; ++i) {
      slots_[i].epoch.store(i, MemoryOrder::SeqCst);
    }
  }

  ~MPSCRingBuffer() noexcept = default;

  MPSCRingBuffer(const MPSCRingBuffer&)            = delete;
  MPSCRingBuffer(MPSCRingBuffer&&)                 = delete;
  MPSCRingBuffer& operator=(const MPSCRingBuffer&) = delete;
  MPSCRingBuffer& operator=(MPSCRingBuffer&&)      = delete;

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
    usize index = read_ % N;
    while (slots_[index].epoch.load(MemoryOrder::Acquire) != read_ + 1) {
      sched_yield();
    }
    T ret(base::move(*slots_[index].value));
    slots_[index].value.destruct();
    slots_[index].epoch.store(read_ + N, MemoryOrder::Release);
    ++read_;
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
    usize index = read_ % N;
    if (slots_[index].epoch.load(MemoryOrder::Acquire) != read_ + 1) {
      return NIL;
    }
    T ret(*slots_[index].value);
    slots_[index].value.destruct();
    slots_[index].epoch.store(read_ + N, MemoryOrder::Release);
    ++read_;
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
