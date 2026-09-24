#pragma once

#include "base/attributes.hpp"
#include "base/export.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/memory/singleton_allocator.hpp"
#include "base/ranges/slice.hpp"
#include "base/type_aliases.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/utils/constants.hpp"

namespace base::memory {

class Arena;
using MainArenaAllocator    = SingletonAllocator<Arena, "main", true>;
using ScratchArenaAllocator = SingletonAllocator<Arena, "scratch", true>;
using PermanentArena =
    SingletonAllocator<Arena, "permanent", true>; // NO NOT RESET

class Arena {
public:
  using SizeT = uptr;

  explicit constexpr Arena(usize capacity) noexcept
      : initial_capacity_((capacity + 4095) / 4096 * 4096) {}
  explicit constexpr Arena() noexcept = default;

  Arena(Arena const& other) noexcept
      : initial_capacity_(other.initial_capacity_) {}
  Arena& operator=(this Arena& self BASE_LIFETIMEBOUND,
                   Arena const&     other) noexcept {
    self.free_all();
    self.initial_capacity_ = other.initial_capacity_;
    return self;
  }

  Arena(Arena&& other) noexcept
      : begin_(other.begin_), end_(other.end_), ptr_(other.ptr_),
        initial_capacity_(other.initial_capacity_) {
    other.begin_ = other.end_ = other.ptr_ = nullptr;
  }
  Arena& operator=(this Arena& self BASE_LIFETIMEBOUND,
                   Arena&&          other) noexcept {
    self.begin_            = other.begin_;
    self.end_              = other.end_;
    self.ptr_              = other.ptr_;
    self.initial_capacity_ = other.initial_capacity_;
    other.begin_ = other.end_ = other.ptr_ = nullptr;
    return self;
  }

  constexpr ~Arena() noexcept = default;

  template <type_traits::TriviallyDestructible T, typename... Args>
  [[nodiscard]] T& push(this Arena& self, Args&&... args);
  template <type_traits::TriviallyDestructible T>
  [[nodiscard]] ranges::Slice<T, SizeT> push_array(this Arena& self,
                                                   SizeT       count);

  template <typename T>
  [[nodiscard]] ranges::Slice<T, SizeT> allocate(this Arena& self,
                                                 SizeT       count) noexcept;
  template <typename T>
  void free(this Arena& /*self*/, ranges::Slice<T, SizeT> /*mem*/) noexcept {}

  [[nodiscard]] constexpr bool operator==(this Arena const& self,
                                          Arena const&      other) noexcept {
    return &self == &other;
  }

public:
  class Scope {
  public:
    explicit Scope(BASE_LIFETIMEBOUND Arena& arena)
        : arena_(arena), ptr_(arena.ptr_) {}
    Scope(Scope const&)     = delete;
    Scope(Scope&&) noexcept = delete;
    ~Scope() noexcept BASE_API;
    Scope& operator=(Scope const&)     = delete;
    Scope& operator=(Scope&&) noexcept = delete;

  private:
    Arena& arena_;
    uchar* ptr_;
  };

private:
  void reserve(this Arena& self, usize capacity) noexcept BASE_API;
  void free_all(this Arena& self) noexcept BASE_API;

private:
  uchar* begin_            = nullptr;
  uchar* end_              = nullptr;
  uchar* ptr_              = nullptr;
  usize  initial_capacity_ = 64 * utils::KiB;
};

template <type_traits::TriviallyDestructible T, typename... Args>
T& Arena::push(this Arena& self, Args&&... args) {
  return *memory::construct_at(self.allocate<T>(1).data(),
                               memory::forward<Args>(args)...);
}

template <type_traits::TriviallyDestructible T>
ranges::Slice<T, Arena::SizeT> Arena::push_array(this Arena& self,
                                                 SizeT       count) {
  ranges::Slice allocated = self.allocate<T>(count);

  for (T& elem : allocated) {
    memory::construct_at(&elem);
  }
  return allocated;
}

// NOLINTNEXTLINE(misc-no-recursion)
template <typename T> // NOLINTNEXTLINE(misc-no-recursion)
ranges::Slice<T, Arena::SizeT> Arena::allocate(this Arena& self,
                                               SizeT       count) noexcept {
  uptr top = reinterpret_cast<uptr>(self.ptr_);
  top      = (top + alignof(T) - 1) & ~(alignof(T) - 1);
  uptr ret = top;
  top += sizeof(T) * count;

  if (top > reinterpret_cast<uptr>(self.end_)) [[unlikely]] {
    self.reserve(static_cast<usize>(self.end_ - self.begin_) * 2 +
                 sizeof(T) * count);

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
template int& Arena::push<int>(
    this Arena& self); // Instantiates to avoid 'placement_new.hpp is unsed'.
#endif

} // namespace base::memory
