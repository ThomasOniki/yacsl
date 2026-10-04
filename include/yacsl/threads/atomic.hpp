#pragma once

#include "yacsl/containers/stack_storage.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::threads {

enum class MemoryOrder : u8 {
  Relaxed = __ATOMIC_RELAXED,
  Consume = __ATOMIC_CONSUME,
  Acquire = __ATOMIC_ACQUIRE,
  Release = __ATOMIC_RELEASE,
  AcqRel  = __ATOMIC_ACQ_REL,
  SeqCst  = __ATOMIC_SEQ_CST,
};

template <typename T>
  requires meta::TriviallyCopyable<T>
class Atomic {
public:
  constexpr Atomic() noexcept = default;

  constexpr Atomic(T value) : value_(value) {}
  Atomic(Atomic const&)     = delete;
  Atomic(Atomic&&) noexcept = delete;

  ~Atomic() noexcept = default;

  Atomic& operator=(this Atomic& self, Atomic const&)     = delete;
  Atomic& operator=(this Atomic& self, Atomic&&) noexcept = delete;

  [[nodiscard]] T load(this Atomic const& self, MemoryOrder order) noexcept {
    containers::StackStorage<T> ret{};
    __atomic_load(&self.value_, &*ret, static_cast<int>(order));
    return *ret;
  }

  void store(this Atomic& self, T value, MemoryOrder order) noexcept {
    __atomic_store(&self.value_, &value, static_cast<int>(order));
  }

  T fetch_add(this Atomic& self, T value, MemoryOrder order) noexcept {
    return __atomic_fetch_add(&self.value_, value, static_cast<int>(order));
  }
  T fetch_sub(this Atomic& self, T value, MemoryOrder order) noexcept {
    return __atomic_fetch_sub(&self.value_, value, static_cast<int>(order));
  }

  T add_fetch(this Atomic& self, T value, MemoryOrder order) noexcept {
    return __atomic_add_fetch(&self.value_, value, static_cast<int>(order));
  }
  T sub_fetch(this Atomic& self, T value, MemoryOrder order) noexcept {
    return __atomic_sub_fetch(&self.value_, value, static_cast<int>(order));
  }

  T exchange(this Atomic& self, T value, MemoryOrder order) noexcept {
    containers::StackStorage<T> ret;
    __atomic_exchange(&self.value_, &value, &*ret, static_cast<int>(order));
    return *ret;
  }

  bool compare_exchange(this Atomic& self, T& expected, T desired,
                        MemoryOrder success, MemoryOrder failure) noexcept {
    return __atomic_compare_exchange(&self.value_, &expected, &desired,
                                     /*weak:*/ false, static_cast<int>(success),
                                     static_cast<int>(failure));
  }
  bool compare_exchange_weak(this Atomic& self, T& expected, T desired,
                             MemoryOrder success,
                             MemoryOrder failure) noexcept {
    return __atomic_compare_exchange(&self.value_, &expected, &desired,
                                     /*weak:*/ true, static_cast<int>(success),
                                     static_cast<int>(failure));
  }

  T operator++(this Atomic& self) noexcept {
    return self.add_fetch(1, MemoryOrder::SeqCst);
  }
  T operator++(this Atomic& self, int) noexcept {
    return self.fetch_add(1, MemoryOrder::SeqCst);
  }
  T operator--(this Atomic& self) noexcept {
    return self.sub_fetch(1, MemoryOrder::SeqCst);
  }
  T operator--(this Atomic& self, int) noexcept {
    return self.fetch_sub(1, MemoryOrder::SeqCst);
  }

private:
  T value_ = T();
};

} // namespace yacsl::threads
