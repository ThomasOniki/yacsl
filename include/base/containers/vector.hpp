#pragma once

#include "base/attributes.hpp"
#include "base/containers/base_classes.hpp"
#include "base/memory/arena.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/initializer_list.hpp"
#include "base/ranges/integer_range.hpp"
#include "base/ranges/slice.hpp"
#include "base/type_traits/allocator.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/comparaison.hpp"
#include "base/type_traits/convertible_to.hpp"
#include "base/type_traits/range_like.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename T, usize N = 0, AllocatorLike Allocator = MainArenaAllocator,
          Integral SizeT = usize>
class Vector : private FixedBufferMember<T, N, SizeT>, AllocatorMember<Allocator> {
public:
  constexpr Vector() noexcept : FixedBufferMember<T, N, SizeT>(), AllocatorMember<Allocator>() {}
  constexpr Vector(InitializerList<T> const& init);

  template <RangeOf<T> Range> constexpr Vector(Range const& range);
  constexpr explicit Vector(SizeT count);

  constexpr ~Vector() noexcept;

  template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>

  constexpr Vector(Vector<T, OtherN, OtherAllocator, OtherSizeT> const& other);
  constexpr Vector(Vector const& other);
  template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>

  constexpr Vector(Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) noexcept;
  constexpr Vector(Vector&& other) noexcept;

  template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
  constexpr Vector& operator=(this Vector&                                         self,
                              Vector<T, OtherN, OtherAllocator, OtherSizeT> const& other);
  constexpr Vector& operator=(this Vector& self, Vector const& other);
  template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
  constexpr Vector& operator=(this Vector&                                    self,
                              Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) noexcept;
  constexpr Vector& operator=(this Vector& self, Vector&& other) noexcept;

  constexpr Vector& operator=(this Vector& self, InitializerList<T> const& init);

  constexpr T& push_back(this Vector& self, T const& value) { return self.emplace_back(value); }
  constexpr T& push_back(this Vector& self, T&& value) {
    return self.emplace_back(base::move(value));
  }

  template <typename... Args> constexpr T& emplace_back(this Vector& self, Args&&... args);

  constexpr void pop_back(this Vector& self) noexcept;
  constexpr void clear(this Vector& self) noexcept;

  constexpr void reserve(this Vector& self, SizeT capacity);
  constexpr void resize(this Vector& self, SizeT count);

  constexpr void shrink_to_fit(this Vector& self) { self.definitely_reserve(self.size_); }

  [[nodiscard]] constexpr SizeT size(this Vector const& self) noexcept { return self.size_; }
  [[nodiscard]] constexpr SizeT capacity(this Vector const& self) noexcept {
    return self.capacity_;
  }

  [[nodiscard]] constexpr T* data(this Vector& self BASE_LIFETIMEBOUND) noexcept {
    return self.data_;
  }
  [[nodiscard]] constexpr T* data(this Vector&& self BASE_LIFETIMEBOUND) noexcept {
    return base::move(self).data_;
  }
  [[nodiscard]] constexpr T const* data(this Vector const& self BASE_LIFETIMEBOUND) noexcept {
    return self.data_;
  }

  [[nodiscard]] constexpr T& operator[](this Vector& self BASE_LIFETIMEBOUND,
                                        SizeT             index) noexcept {
    return self.data_[static_cast<usize>(index)];
  }
  [[nodiscard]] constexpr T&& operator[](this Vector&& self BASE_LIFETIMEBOUND,
                                         SizeT              index) noexcept {
    return base::move(base::move(self).data_[static_cast<usize>(index)]);
  }
  [[nodiscard]] constexpr T const& operator[](this Vector const& self BASE_LIFETIMEBOUND,
                                              SizeT                   index) noexcept {
    return self.data_[static_cast<usize>(index)];
  }
  [[nodiscard]] constexpr T const&& operator[](this Vector const&& self BASE_LIFETIMEBOUND,
                                               SizeT                    index) noexcept {
    return base::move(base::move(self).data_[static_cast<usize>(index)]);
  }

  template <typename Self>
  [[nodiscard]] constexpr auto slice(this Self&& self BASE_LIFETIMEBOUND) noexcept
      -> Slice<RemoveReference<decltype(*self.begin())>, SizeT> {
    return {base::forward<Self>(self).data_, base::forward<Self>(self).size_};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto reversed_slice(this Self&& self BASE_LIFETIMEBOUND) noexcept
      -> ReversedSlice<RemoveReference<decltype(*self.begin())>, SizeT> {
    return {base::forward<Self>(self).data_, base::forward<Self>(self).size_};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto bytes(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).slice().bytes();
  }

  [[nodiscard]] constexpr IntegerRange<SizeT> indices(this Vector const& self) noexcept {
    return {0, self.size_};
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&& front(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self)[0];
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& back(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self)[self.size_ - 1];
  }

  template <typename Self>
  [[nodiscard]] constexpr auto begin(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto end(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_ + base::forward<Self>(self).size_;
  }

private:
  void maybe_alloc_size();          // if size_ > N
  void maybe_free_and_alloc_size(); // if size_ > current capacity

  void alloc(SizeT capacity);
  void free();

  template <typename Copied> void copy(Copied const* data);
  void                            move(T* data);

  void definitely_reserve(SizeT capacity);

  template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
  void steal_ptr(Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other);

private:
  template <typename OtherT, usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
  friend class Vector;

  using FixedBufferMember<T, N, SizeT>::fixed_buffer_;
  using AllocatorMember<Allocator>::allocator;

  T*    data_     = fixed_buffer_.data();
  SizeT size_     = 0;
  SizeT capacity_ = fixed_buffer_.size();
};

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(InitializerList<T> const& init)
    : size_(static_cast<SizeT>(init.size())) {
  maybe_alloc_size();
  copy(init.begin());
}
template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
template <RangeOf<T> Range>
constexpr Vector<T, N, Allocator, SizeT>::Vector(Range const& range) {
  if constexpr (requires(Range with_size) {
                  { with_size.size() } -> Integral;
                }) {
    size_ = range.size();
    maybe_alloc_size();
    if constexpr (requires(Range with_ptr) {
                    { with_ptr.data() } -> OneOf<T*, T const*>;
                  }) {
      copy(range.data());
    } else if constexpr (requires(Range with_ptr) {
                           { with_ptr.begin() } -> OneOf<T*, T const*>;
                         }) {
      copy(range.begin());
    } else if constexpr (requires(Range indexable) {
                           { indexable[0] } -> ConvertibleTo<T>;
                         }) {
      for (SizeT i = 0; i < size_; ++i) {
        base::construct_at(data_ + i, range[i]);
      }
    } else {
      auto iter = range.begin();
      for (SizeT i = 0; i < size_; ++iter, ++i) {
        base::construct_at(data_ + i, *iter);
      }
    }
  } else {
    for (auto&& element : range) {
      emplace_back(base::forward<decltype(element)>(element));
    }
  }
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(SizeT count) : size_(count) {
  maybe_alloc_size();
  for (SizeT i = 0; i < count; ++i) {
    base::construct_at(data_ + i);
  }
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>::~Vector() noexcept {
  clear();
  free();
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(
    Vector<T, OtherN, OtherAllocator, OtherSizeT> const& other)
    : size_(static_cast<SizeT>(other.size_)) {
  maybe_alloc_size();
  copy(other.data_);
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(Vector const& other)
    : size_(static_cast<SizeT>(other.size_)) {
  maybe_alloc_size();
  copy(other.data_);
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(
    Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) noexcept
    : size_(static_cast<SizeT>(other.size_)) {
  if constexpr (!SingletonAllocatorLike<Allocator> || !SameAs<Allocator, OtherAllocator>) {
    maybe_alloc_size();
    move(other.data_);
    return;
  }

  if constexpr (OtherN > 0) {
    if (other.data_ == other.fixed_buffer_.data()) {
      if constexpr (OtherN > N) {
        maybe_alloc_size();
      }
      move(other.data_);
      return;
    }
  }
  steal_ptr(base::move(other));
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(Vector&& other) noexcept
    : size_(static_cast<SizeT>(other.size_)) {
  if constexpr (N > 0) {
    if (other.data_ == other.fixed_buffer_.data()) {
      move(other.data_);
      return;
    }
  }
  steal_ptr(base::move(other));
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
constexpr Vector<T, N, Allocator, SizeT>& Vector<T, N, Allocator, SizeT>::operator=(
    this Vector& self, Vector<T, OtherN, OtherAllocator, OtherSizeT> const& other) {
  self.clear();
  self.size_ = static_cast<SizeT>(other.size_);
  self.reserve(self.size_);
  self.copy(other.data_);
  return self;
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(this Vector& self, Vector const& other) {
  if (&other == &self) [[unlikely]] {
    return self;
  }
  self.clear();
  self.size_ = static_cast<SizeT>(other.size_);
  self.reserve(self.size_);
  self.copy(other.data_);
  return self;
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
constexpr Vector<T, N, Allocator, SizeT>& Vector<T, N, Allocator, SizeT>::operator=(
    this Vector& self, Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) noexcept {
  self.clear();
  self.size_ = static_cast<SizeT>(other.size_);

  if constexpr (!SingletonAllocatorLike<Allocator> || !SameAs<Allocator, OtherAllocator>) {
    self.reserve(self.size_);
    self.move(other.data_);
    return self;
  }

  if constexpr (OtherN > 0) {
    if (other.data_ == other.fixed_buffer_.data()) {
      if constexpr (OtherN > N) {
        self.reserve(self.size_);
      }
      self.move(other.data_);
      return self;
    }
  }
  self.free();
  self.steal_ptr(base::move(other));
  return self;
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(this Vector& self, Vector&& other) noexcept {
  self.clear();
  self.size_ = static_cast<SizeT>(other.size_);

  if constexpr (N > 0) {
    if (other.data_ == other.fixed_buffer_.data()) {
      self.move(other.data_);
      return self;
    }
  }
  self.free();
  self.steal_ptr(base::move(other));
  return self;
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(this Vector& self, InitializerList<T> const& init) {
  self.clear();
  self.size_ = static_cast<SizeT>(init.size());
  self.reserve(self.size_);
  self.copy(init.begin());
  return self;
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
template <typename... Args>
constexpr T& Vector<T, N, Allocator, SizeT>::emplace_back(this Vector& self, Args&&... args) {
  if (self.size_ == self.capacity_) [[unlikely]] {
    self.definitely_reserve(static_cast<SizeT>(self.capacity_ * 2 + 1));
  }
  return *base::construct_at(self.data_ + self.size_++, base::forward<Args>(args)...);
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::pop_back(this Vector& self) noexcept {
  self.data_[--self.size_].~T();
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::clear(this Vector& self) noexcept {
  if constexpr (!TriviallyDestructible<T>) {
    while (self.size_ > 0) {
      self.pop_back();
    }
  } else {
    self.size_ = 0;
  }
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::reserve(this Vector& self, SizeT capacity) {
  if (capacity > self.capacity_) {
    self.definitely_reserve(capacity);
  }
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::resize(this Vector& self, SizeT count) {
  if (self.size_ >= count) {
    if constexpr (TriviallyDestructible<T>) {
      self.size_ = count;
    } else {
      while (self.size_ > count) {
        self.pop_back();
      }
    }
  } else {
    self.reserve(count);
    for (; self.size_ < count; ++self.size_) {
      base::construct_at(&self.data_[self.size_]);
    }
  }
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
void Vector<T, N, Allocator, SizeT>::maybe_alloc_size() {
  if constexpr (N > 0) {
    if (size_ > static_cast<SizeT>(N)) {
      alloc(size_);
    }
  } else {
    alloc(size_);
  }
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
void Vector<T, N, Allocator, SizeT>::alloc(SizeT capacity) {
  capacity_ = capacity;
  data_     = allocator().template allocate<T>(static_cast<Allocator::SizeT>(capacity_)).data();
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
void Vector<T, N, Allocator, SizeT>::free() {
  if constexpr (N > 0) {
    if (data_ != fixed_buffer_.data()) {
      allocator().template free<T>({data_, static_cast<Allocator::SizeT>(capacity_)});
    }
  } else {
    allocator().template free<T>({data_, static_cast<Allocator::SizeT>(capacity_)});
  }
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
void Vector<T, N, Allocator, SizeT>::definitely_reserve(SizeT capacity) {
  T* old_data = data_;
  data_       = allocator().template allocate<T>(static_cast<Allocator::SizeT>(capacity)).data();
  move(old_data);

  for (SizeT i = 0; i < size_; ++i) {
    old_data[i].~T();
  }

  if constexpr (N > 0) {
    if (old_data != fixed_buffer_.data()) {
      allocator().template free<T>({old_data, static_cast<Allocator::SizeT>(capacity_)});
    }
  } else {
    allocator().template free<T>({old_data, static_cast<Allocator::SizeT>(capacity_)});
  }
  capacity_ = capacity;
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
template <typename Copied>
void Vector<T, N, Allocator, SizeT>::copy(Copied const* data) {
  if constexpr (TriviallyCopyable<T> && SameAs<T, Copied>) {
    base::copy({data, static_cast<usize>(size_) * sizeof(T)}, data_);
  } else {
    for (SizeT i = 0; i < size_; ++i) {
      base::construct_at(data_ + i, data[i]);
    }
  }
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
void Vector<T, N, Allocator, SizeT>::move(T* data) {
  if constexpr (TriviallyCopyable<T>) {
    if (size_ > 0) {
      base::copy({data, static_cast<usize>(size_) * sizeof(T)}, data_);
    }
  } else {
    for (SizeT i = 0; i < size_; ++i) {
      base::construct_at(data_ + i, base::move(data[i]));
    }
  }
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
void Vector<T, N, Allocator, SizeT>::steal_ptr(
    Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) {
  data_       = other.data_;
  capacity_   = static_cast<SizeT>(other.capacity_);
  other.size_ = 0;
  other.data_ = nullptr;

  (void)base::move(other);
}

} // namespace base

#ifdef BASE_LSP_ONLY
template class base::Vector<int>; // Instantiates to avoid 'arena.hpp is unused'.
#endif
