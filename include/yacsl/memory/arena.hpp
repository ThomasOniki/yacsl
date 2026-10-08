#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/compiler/export.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/memory/singleton_allocator.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/utils/constants.hpp"

namespace yacsl::memory {

class Arena {
public:
  using SizeT = uptr;

  explicit constexpr Arena(usize capacity) noexcept
      : initial_capacity_((capacity + 4095) / 4096 * 4096) {}
  explicit constexpr Arena() noexcept = default;

  Arena(Arena const& other) noexcept
      : initial_capacity_(other.initial_capacity_) {}
  Arena& operator=(this Arena& self YACSL_LIFETIMEBOUND,
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
  Arena& operator=(this Arena& self YACSL_LIFETIMEBOUND,
                   Arena&&          other) noexcept {
    self.begin_            = other.begin_;
    self.end_              = other.end_;
    self.ptr_              = other.ptr_;
    self.initial_capacity_ = other.initial_capacity_;
    other.begin_ = other.end_ = other.ptr_ = nullptr;
    return self;
  }

  constexpr ~Arena() noexcept = default;

  template <meta::TriviallyDestructible T, typename... Args>
  [[nodiscard]] T& push(this Arena& self, Args&&... args);
  template <meta::TriviallyDestructible T>
  [[nodiscard]] ranges::Slice<T, SizeT> push_array(this Arena& self,
                                                   SizeT       count);

  [[nodiscard]] ranges::Slice<u8, SizeT> allocate(this Arena& self, SizeT size,
                                                  Alignment align) noexcept {
    auto const u_align = static_cast<usize>(align);
    uptr       top     = reinterpret_cast<uptr>(self.ptr_);
    top                = (top + u_align - 1) / u_align * u_align;
    uptr ret           = top;
    top += size;

    if (top > reinterpret_cast<uptr>(self.end_)) [[unlikely]] {
      self.reserve(static_cast<usize>(self.end_ - self.begin_) * 2 + size);

      top = reinterpret_cast<uptr>(self.ptr_);
      top = (top + u_align - 1) / u_align * u_align;
      ret = top;
      top += size;
    }

    // NOLINTNEXTLINE(performance-no-int-to-ptr)
    self.ptr_ = reinterpret_cast<u8*>(top);
    // NOLINTNEXTLINE(performance-no-int-to-ptr)
    return {reinterpret_cast<u8*>(ret), size};
  }

  void deallocate(this Arena& self, ranges::Slice<u8, SizeT> data) noexcept {
    if (data.end() == self.end_) {
      self.ptr_ = data.begin();
    }
  }

  [[nodiscard]] ranges::Slice<u8, SizeT>
  resize_in_place(ranges::Slice<u8, SizeT> data, SizeT new_size) noexcept {
    u8* new_ptr = ptr_ + new_size - data.len();
    if (data.end() == ptr_ && new_ptr <= end_) {
      ptr_ = new_ptr;
      return {data.data(), new_size};
    }
    return {nullptr, 0UZ};
  }

  [[nodiscard]] constexpr bool operator==(this Arena const& self,
                                          Arena const&      other) noexcept {
    return &self == &other;
  }

public:
  class Scope {
  public:
    explicit Scope(YACSL_LIFETIMEBOUND Arena& arena)
        : arena_(arena), ptr_(arena.ptr_) {}
    Scope(Scope const&)     = delete;
    Scope(Scope&&) noexcept = delete;
    ~Scope() noexcept YACSL_API;
    Scope& operator=(Scope const&)     = delete;
    Scope& operator=(Scope&&) noexcept = delete;

  private:
    Arena& arena_;
    u8*    ptr_;
  };

private:
  void reserve(this Arena& self, usize capacity) noexcept YACSL_API;
  void free_all(this Arena& self) noexcept YACSL_API;

private:
  u8*   begin_            = nullptr;
  u8*   end_              = nullptr;
  u8*   ptr_              = nullptr;
  usize initial_capacity_ = 64 * utils::KiB;
};

using MainArenaAllocator    = SingletonAllocator<Arena, "main", true>;
using ScratchArenaAllocator = SingletonAllocator<Arena, "scratch", true>;
using PermanentArena =
    SingletonAllocator<Arena, "permanent", true>; // NO NOT RESET

template <meta::TriviallyDestructible T, typename... Args>
T& Arena::push(this Arena& self, Args&&... args) {
  // bytes has to be mutable to not cast away const in the reinterpret_cast
  // NOLINTNEXTLINE(misc-const-correctness)
  auto bytes = self.allocate(sizeof(T), memory::align_of<T>);
  return *memory::construct_at(reinterpret_cast<T*>(bytes.data()),
                               memory::forward<Args>(args)...);
}

template <meta::TriviallyDestructible T>
ranges::Slice<T, Arena::SizeT> Arena::push_array(this Arena& self,
                                                 SizeT       count) {
  // bytes has to be mutable to not cast away const in the reinterpret_cast
  // NOLINTNEXTLINE(misc-const-correctness)
  auto  bytes     = self.allocate(sizeof(T) * count, memory::align_of<T>);
  auto* allocated = reinterpret_cast<T*>(bytes.data());

  for (SizeT i = 0; i < count; ++i) {
    memory::construct_at(allocated + i);
  }
  return {allocated, count};
}

#ifdef YACSL_LSP_ONLY
template int& Arena::push<int>(
    this Arena& self); // Instantiates to avoid 'placement_new.hpp is unsed'.
#endif

} // namespace yacsl::memory
