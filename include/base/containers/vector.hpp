#pragma once

#include "base/attributes.hpp"
#include "base/containers/base_classes.hpp"
#include "base/memory/arena.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/initializer_list.hpp"
#include "base/ranges/integer_range.hpp"
#include "base/ranges/reverse_iterator.hpp"
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
  constexpr Vector(const InitializerList<T>& init);
  // NOLINTNEXTLINE(google-explicit-constructor)
  template <RangeOf<T> Range> constexpr Vector(const Range& range);
  constexpr explicit Vector(SizeT count);

  constexpr ~Vector() noexcept;

  template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Vector(const Vector<T, OtherN, OtherAllocator, OtherSizeT>& other);
  constexpr Vector(const Vector& other);
  template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Vector(Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) noexcept;
  constexpr Vector(Vector&& other) noexcept;

  template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
  constexpr Vector& operator=(const Vector<T, OtherN, OtherAllocator, OtherSizeT>& other);
  constexpr Vector& operator=(const Vector& other);
  template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
  constexpr Vector& operator=(Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) noexcept;
  constexpr Vector& operator=(Vector&& other) noexcept;

  constexpr Vector& operator=(const InitializerList<T>& init);

  constexpr T& push_back(const T& value) { return emplace_back(value); }
  constexpr T& push_back(T&& value) { return emplace_back(base::move(value)); }

  template <typename... Args> constexpr T& emplace_back(Args&&... args);

  constexpr void pop_back() noexcept;
  constexpr void clear() noexcept;

  constexpr void reserve(SizeT capacity);
  constexpr void resize(SizeT count);

  constexpr void shrink_to_fit() { definitely_reserve(size_); }

  [[nodiscard]] constexpr SizeT size() const noexcept { return size_; }
  [[nodiscard]] constexpr SizeT capacity() const noexcept { return capacity_; }

  [[nodiscard]] constexpr T*       data() noexcept BASE_LIFETIMEBOUND { return data_; }
  [[nodiscard]] constexpr const T* data() const noexcept BASE_LIFETIMEBOUND { return data_; }

  [[nodiscard]] constexpr T& operator[](SizeT index) noexcept BASE_LIFETIMEBOUND {
    return data_[static_cast<usize>(index)];
  }
  [[nodiscard]] constexpr T& operator[](SizeT index) const noexcept BASE_LIFETIMEBOUND {
    return data_[static_cast<usize>(index)];
  }

  [[nodiscard]] constexpr Slice<T, SizeT> slice() noexcept BASE_LIFETIMEBOUND {
    return {data_, size_};
  }
  [[nodiscard]] constexpr Slice<const T, SizeT> slice() const noexcept BASE_LIFETIMEBOUND {
    return {data_, size_};
  }
  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<uchar*>(data_), static_cast<SizeT>(size_ * sizeof(T))};
  }
  [[nodiscard]] constexpr Slice<const uchar, SizeT> bytes() const noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const uchar*>(data_), static_cast<SizeT>(size_ * sizeof(T))};
  }

  [[nodiscard]] constexpr IntegerRange<SizeT> indices() const noexcept { return {0, size_}; }

  [[nodiscard]] constexpr T&       front() noexcept BASE_LIFETIMEBOUND { return data_[0]; }
  [[nodiscard]] constexpr const T& front() const noexcept BASE_LIFETIMEBOUND { return data_[0]; }
  [[nodiscard]] constexpr T&       back() noexcept BASE_LIFETIMEBOUND { return data_[size_ - 1]; }
  [[nodiscard]] constexpr const T& back() const noexcept BASE_LIFETIMEBOUND {
    return data_[size_ - 1];
  }

  [[nodiscard]] constexpr T*       begin() noexcept BASE_LIFETIMEBOUND { return data_; }
  [[nodiscard]] constexpr const T* begin() const noexcept BASE_LIFETIMEBOUND { return data_; }
  [[nodiscard]] constexpr T*       end() noexcept BASE_LIFETIMEBOUND { return data_ + size_; }
  [[nodiscard]] constexpr const T* end() const noexcept BASE_LIFETIMEBOUND { return data_ + size_; }

  [[nodiscard]] constexpr ReverseIterator<T*> rbegin() noexcept BASE_LIFETIMEBOUND {
    return data_ + size_;
  }
  [[nodiscard]] constexpr ReverseIterator<const T*> rbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_ + size_;
  }
  [[nodiscard]] constexpr ReverseIterator<T*> rend() noexcept BASE_LIFETIMEBOUND { return data_; }
  [[nodiscard]] constexpr ReverseIterator<const T*> rend() const noexcept BASE_LIFETIMEBOUND {
    return data_;
  }

  [[nodiscard]] constexpr const T* cbegin() const noexcept BASE_LIFETIMEBOUND { return data_; }
  [[nodiscard]] constexpr const T* cend() const noexcept BASE_LIFETIMEBOUND {
    return data_ + size_;
  }

  [[nodiscard]] constexpr ReverseIterator<const T*> crbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_ + size_;
  }
  [[nodiscard]] constexpr ReverseIterator<const T*> crend() const noexcept BASE_LIFETIMEBOUND {
    return data_;
  }

private:
  void maybe_alloc_size();          // if size_ > N
  void maybe_free_and_alloc_size(); // if size_ > current capacity

  void alloc(SizeT capacity);
  void free();

  template <typename Copied> void copy(const Copied* data);
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
constexpr Vector<T, N, Allocator, SizeT>::Vector(const InitializerList<T>& init)
    : size_(static_cast<SizeT>(init.size())) {
  maybe_alloc_size();
  copy(init.begin());
}
template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
template <RangeOf<T> Range>
constexpr Vector<T, N, Allocator, SizeT>::Vector(const Range& range) {
  if constexpr (requires(Range with_size) {
                  { with_size.size() } -> Integral;
                }) {
    size_ = range.size();
    maybe_alloc_size();
    if constexpr (requires(Range with_ptr) {
                    { with_ptr.data() } -> OneOf<T*, const T*>;
                  }) {
      copy(range.data());
    } else if constexpr (requires(Range with_ptr) {
                           { with_ptr.begin() } -> OneOf<T*, const T*>;
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
    const Vector<T, OtherN, OtherAllocator, OtherSizeT>& other)
    : size_(static_cast<SizeT>(other.size_)) {
  maybe_alloc_size();
  copy(other.data_);
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(const Vector& other)
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
    const Vector<T, OtherN, OtherAllocator, OtherSizeT>& other) {
  clear();
  size_ = static_cast<SizeT>(other.size_);
  reserve(size_);
  copy(other.data_);
  return *this;
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(const Vector& other) {
  if (&other == this) [[unlikely]] {
    return *this;
  }
  clear();
  size_ = static_cast<SizeT>(other.size_);
  reserve(size_);
  copy(other.data_);
  return *this;
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
template <usize OtherN, AllocatorLike OtherAllocator, Integral OtherSizeT>
constexpr Vector<T, N, Allocator, SizeT>& Vector<T, N, Allocator, SizeT>::operator=(
    Vector<T, OtherN, OtherAllocator, OtherSizeT>&& other) noexcept {
  clear();
  size_ = static_cast<SizeT>(other.size_);

  if constexpr (!SingletonAllocatorLike<Allocator> || !SameAs<Allocator, OtherAllocator>) {
    reserve(size_);
    move(other.data_);
    return *this;
  }

  if constexpr (OtherN > 0) {
    if (other.data_ == other.fixed_buffer_.data()) {
      if constexpr (OtherN > N) {
        reserve(size_);
      }
      move(other.data_);
      return *this;
    }
  }
  free();
  steal_ptr(base::move(other));
  return *this;
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(Vector&& other) noexcept {
  clear();
  size_ = static_cast<SizeT>(other.size_);

  if constexpr (N > 0) {
    if (other.data_ == other.fixed_buffer_.data()) {
      move(other.data_);
      return *this;
    }
  }
  free();
  steal_ptr(base::move(other));
  return *this;
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(const InitializerList<T>& init) {
  clear();
  size_ = static_cast<SizeT>(init.size());
  reserve(size_);
  copy(init.begin());
  return *this;
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
template <typename... Args>
constexpr T& Vector<T, N, Allocator, SizeT>::emplace_back(Args&&... args) {
  if (size_ == capacity_) [[unlikely]] {
    definitely_reserve(static_cast<SizeT>(capacity_ * 2 + 1));
  }
  return *base::construct_at(data_ + size_++, base::forward<Args>(args)...);
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::pop_back() noexcept {
  --size_;
  data_[size_].~T();
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::clear() noexcept {
  if constexpr (!TriviallyDestructible<T>) {
    while (size_ > 0) {
      pop_back();
    }
  } else {
    size_ = 0;
  }
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::reserve(SizeT capacity) {
  if (capacity > capacity_) {
    definitely_reserve(capacity);
  }
}

template <typename T, usize N, AllocatorLike Allocator, Integral SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::resize(SizeT count) {
  if (size_ >= count) {
    if constexpr (TriviallyDestructible<T>) {
      size_ = count;
    } else {
      while (size_ > count) {
        pop_back();
      }
    }
  } else {
    reserve(count);
    for (; size_ < count; ++size_) {
      base::construct_at(&data_[size_]);
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
void Vector<T, N, Allocator, SizeT>::copy(const Copied* data) {
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
