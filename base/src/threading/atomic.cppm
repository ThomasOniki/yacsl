export module base.threading.atomic;

import base.type_traits.trivial;
import base.containers.stack_storage;

export namespace base {

// NOLINTNEXTLINE(performance-enum-size)
enum class MemoryOrder {
  Relaxed = __ATOMIC_RELAXED,
  Consume = __ATOMIC_CONSUME,
  Acquire = __ATOMIC_ACQUIRE,
  Release = __ATOMIC_RELEASE,
  AcqRel  = __ATOMIC_ACQ_REL,
  SeqCst  = __ATOMIC_SEQ_CST
};

template <typename T>
  requires TriviallyCopyable<T>
class Atomic {
public:
  constexpr Atomic() noexcept = default;
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Atomic(T value) : value_(value) {}
  Atomic(const Atomic&)     = delete;
  Atomic(Atomic&&) noexcept = delete;

  ~Atomic() noexcept = default;

  Atomic& operator=(const Atomic&)     = delete;
  Atomic& operator=(Atomic&&) noexcept = delete;

  [[nodiscard]] T load(MemoryOrder order) const noexcept {
    StackStorage<T> ret;
    __atomic_load(&value_, &*ret, static_cast<int>(order));
    return *ret;
  }

  void store(T value, MemoryOrder order) noexcept {
    __atomic_store(&value_, &value, static_cast<int>(order));
  }

  T fetch_add(T value, MemoryOrder order) noexcept {
    return __atomic_fetch_add(&value_, value, static_cast<int>(order));
  }
  T fetch_sub(T value, MemoryOrder order) noexcept {
    return __atomic_fetch_sub(&value_, value, static_cast<int>(order));
  }

  T exchange(T value, MemoryOrder order) noexcept {
    StackStorage<T> ret;
    __atomic_exchange(&value_, &value, &*ret, static_cast<int>(order));
    return *ret;
  }

  T operator++() noexcept { return fetch_add(1, MemoryOrder::SeqCst) + 1; }
  T operator++(int) noexcept { return fetch_add(1, MemoryOrder::SeqCst); }
  T operator--() noexcept { return fetch_sub(1, MemoryOrder::SeqCst) - 1; }
  T operator--(int) noexcept { return fetch_sub(1, MemoryOrder::SeqCst); }

private:
  T value_ = T();
};

} // namespace base
