#pragma once

#include "base/attributes.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/memory/singleton_allocator.hpp"
#include "base/ranges/slice.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/utilities/constants.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

class Arena;
using MainArenaAllocator    = SingletonAllocator<Arena, "main", true>;
using ScratchArenaAllocator = SingletonAllocator<Arena, "scratch", true>;
using PermanentArena        = SingletonAllocator<Arena, "permanent", true>; // NO NOT RESET

class Arena {
public:
  using SizeT = uptr;

  explicit constexpr Arena(usize capacity) : initial_capacity_((capacity + 4095) / 4096 * 4096) {}
  explicit constexpr Arena() = default;

  Arena(Arena const&)     = delete;
  Arena(Arena&&) noexcept = delete;

  constexpr ~Arena() noexcept = default;

  Arena& operator=(Arena const&)     = delete;
  Arena& operator=(Arena&&) noexcept = delete;

  template <TriviallyDestructible T, typename... Args>
  [[nodiscard]] T& push(this Arena& self, Args&&... args);
  template <TriviallyDestructible T>
  [[nodiscard]] Slice<T, SizeT> push_array(this Arena& self, SizeT count);

  template <typename T>
  [[nodiscard]] Slice<T, SizeT> allocate(this Arena& self, SizeT count) noexcept;
  template <typename T> void    free(this Arena& /*self*/, Slice<T, SizeT> /*mem*/) noexcept {}

public:
  class Scope {
  public:
    explicit Scope(BASE_LIFETIMEBOUND Arena& arena) : arena_(arena), ptr_(arena.ptr_) {}
    Scope(Scope const&)     = delete;
    Scope(Scope&&) noexcept = delete;
    ~Scope() noexcept;
    Scope& operator=(Scope const&)     = delete;
    Scope& operator=(Scope&&) noexcept = delete;

  private:
    Arena& arena_;
    uchar* ptr_;
  };

private:
  void reserve(this Arena& self, usize capacity) noexcept;

private:
  uchar* begin_            = nullptr;
  uchar* end_              = nullptr;
  uchar* ptr_              = nullptr;
  usize  initial_capacity_ = 64 * KiB;
};

template <TriviallyDestructible T, typename... Args>
T& Arena::push(this Arena& self, Args&&... args) {
  return *base::construct_at(self.allocate<T>(1).data(), base::forward<Args>(args)...);
}

template <TriviallyDestructible T>
Slice<T, Arena::SizeT> Arena::push_array(this Arena& self, SizeT count) {
  Slice allocated = self.allocate<T>(count);

  for (T& elem : allocated) {
    base::construct_at(&elem);
  }
  return allocated;
}

// NOLINTNEXTLINE(misc-no-recursion)
template <typename T>
Slice<T, Arena::SizeT> Arena::allocate(this Arena& self, SizeT count) noexcept {
  uptr top = reinterpret_cast<uptr>(self.ptr_);
  top      = (top + alignof(T) - 1) & ~(alignof(T) - 1);
  uptr ret = top;
  top += sizeof(T) * count;

  if (top > reinterpret_cast<uptr>(self.end_)) [[unlikely]] {
    self.reserve(static_cast<usize>(self.end_ - self.begin_) * 2 + sizeof(T) * count);

    top = reinterpret_cast<uptr>(self.ptr_);
    top = (top + alignof(T) - 1) & ~(alignof(T) - 1);
    ret = top;
    top += sizeof(T) * count;
  }

  // NOLINTNEXTLINE(performance-no-int-to-ptr)
  self.ptr_ = reinterpret_cast<uchar*>(top);
  // NOLINTNEXTLINE(performance-no-int-to-ptr)
  return {reinterpret_cast<T*>(ret), count};
}

#ifdef BASE_LSP_ONLY
template int&
Arena::push<int>(this Arena& self); // Instantiates to avoid 'placement_new.hpp is unsed'.
#endif

} // namespace base
