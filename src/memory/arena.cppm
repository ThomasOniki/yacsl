module;

#include "base/attributes.hpp"

export module base.memory.arena;

import base.utilities.type_aliases;
import base.memory.singleton_allocator;
import base.type_traits.trivial;
import base.utilities.type_aliases;
import base.memory.lifetimes;
import base.utilities.constants;

export namespace base {

class Arena;
using MainArenaAllocator    = SingletonAllocator<Arena, "main", true>;
using ScratchArenaAllocator = SingletonAllocator<Arena, "scratch", true>;
using PermanentArena        = SingletonAllocator<Arena, "permanent", true>; // NO NOT RESET

class Arena {
public:
  using SizeT = uptr;

  explicit constexpr Arena(usize capacity) : initial_capacity_((capacity + 4095) / 4096 * 4096) {}
  explicit constexpr Arena() = default;

  Arena(const Arena&)     = delete;
  Arena(Arena&&) noexcept = delete;

  constexpr ~Arena() noexcept = default;

  Arena& operator=(const Arena&)     = delete;
  Arena& operator=(Arena&&) noexcept = delete;

  template <TriviallyDestructible T, typename... Args> [[nodiscard]] T& push(Args&&... args);
  template <TriviallyDestructible T> [[nodiscard]] T*                   push_array(SizeT count);

  template <typename T> [[nodiscard]] T* allocate(SizeT count) noexcept;
  template <typename T> void             free(T* /*ptr*/, SizeT /*count*/) {}

public:
  class Scope {
  public:
    explicit Scope(BASE_LIFETIMEBOUND Arena& arena) : arena_(arena), ptr_(arena.ptr_) {}
    Scope(const Scope&)     = delete;
    Scope(Scope&&) noexcept = delete;
    ~Scope() noexcept;
    Scope& operator=(const Scope&)     = delete;
    Scope& operator=(Scope&&) noexcept = delete;

  private:
    Arena& arena_;
    uchar* ptr_;
  };

private:
  void reserve(usize capacity) noexcept;

private:
  uchar* begin_            = nullptr;
  uchar* end_              = nullptr;
  uchar* ptr_              = nullptr;
  usize  initial_capacity_ = 64 * KiB;
};

template <TriviallyDestructible T, typename... Args> T& Arena::push(Args&&... args) {
  return *base::construct_at(allocate<T>(1), base::forward<Args>(args)...);
}

template <TriviallyDestructible T> T* Arena::push_array(SizeT count) {
  T* ptr = allocate<T>(count);

  for (SizeT i = 0; i < count; ++i) {
    base::construct_at(ptr + i);
  }
  return ptr;
}

template <typename T> T* Arena::allocate(SizeT count) noexcept {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  uptr top = reinterpret_cast<uptr>(ptr_);
  top      = (top + alignof(T) - 1) & ~(alignof(T) - 1);
  uptr ret = top;
  top += sizeof(T) * count;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (top > reinterpret_cast<uptr>(end_)) [[unlikely]] {
    reserve(static_cast<usize>(end_ - begin_) * 2 + sizeof(T) * count);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    top = reinterpret_cast<uptr>(ptr_);
    top = (top + alignof(T) - 1) & ~(alignof(T) - 1);
    ret = top;
    top += sizeof(T) * count;
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
  ptr_ = reinterpret_cast<uchar*>(top);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
  return reinterpret_cast<T*>(ret);
}

#ifdef BASE_LSP_ONLY
template int& Arena::push<int>(); // Instantiates to avoid 'placement_new.hpp is unsed'.
#endif

} // namespace base
