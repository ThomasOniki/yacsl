#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/threads/atomic.hpp"
#include "yacsl/type_traits/allocator.hpp"

namespace yacsl::containers {

template <typename T, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT     = usize,
          auto                  Increment = [](SizeT& count) { ++count; },
          auto Decrement = [](SizeT& count) { return --count; }>
class RefCounted {
public:
  RefCounted(T const& value) : RefCounted(value, Allocator()) {}
  RefCounted(T&& value) : RefCounted(memory::move(value), Allocator()) {}

  explicit RefCounted(T const& value, Allocator allocator)
      : ptr_(allocator.template allocate<Data>(1).data()) {
    memory::construct_at(ptr_, value, 1, memory::move(allocator));
  }
  explicit RefCounted(T&& value, Allocator allocator)
      : ptr_(allocator.template allocate<Data>(1).data()) {
    memory::construct_at(ptr_, memory::move(value), 1, memory::move(allocator));
  }

  RefCounted(RefCounted const& other) noexcept : ptr_(other.ptr_) {
    Increment(ptr_->count);
  }
  RefCounted(RefCounted&& other) noexcept : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }
  RefCounted& operator=(this RefCounted&  self,
                        RefCounted const& other) noexcept {
    self.release();
    self.ptr_ = other.ptr_;
    Increment(self.ptr_->count);
  }
  RefCounted& operator=(this RefCounted& self, RefCounted&& other) noexcept {
    self.release();
    self.ptr_  = other.ptr_;
    other.ptr_ = nullptr;
  }

  ~RefCounted() noexcept { release(); }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  get(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<type_traits::CopyCVRef<Self&&, T>>(
        *memory::forward<Self>(self).ptr_);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  operator*(this Self&& self YACSL_LIFETIMEBOUND) {
    return memory::forward<type_traits::CopyCVRef<Self&&, T>>(
        *memory::forward<Self>(self).ptr_);
  }
  template <typename Self>
  [[nodiscard]] constexpr type_traits::CopyCV<Self, T>*
  operator->(this Self&& self YACSL_LIFETIMEBOUND) {
    return memory::forward<Self>(self).ptr_;
  }

private:
  void release(this RefCounted& self) {
    if (self.ptr_ == nullptr) {
      return;
    }
    if (Decrement(self.ptr_->count) > 0) {
      return;
    }
    Allocator alloc = memory::move(self.ptr_->alloc);
    self.ptr_->~Data();
    alloc.template free<Data>(self.ptr_);
  }

private:
  struct Data {
    T                                value;
    SizeT                            count;
    YACSL_NO_UNIQUE_ADDRESS Allocator alloc;
  };
  Data* ptr_;
};

template <typename T, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT = usize>
using AtomicRefCounted =
    RefCounted<T, Allocator, threads::Atomic<SizeT>,
               [](threads::Atomic<SizeT>& count) {
                 count.fetch_add(1, threads::MemoryOrder::Relaxed);
               },
               [](threads::Atomic<SizeT>& count) {
                 return count.sub_fetch(1, threads::MemoryOrder::AcqRel);
               }>;

} // namespace yacsl::containers
