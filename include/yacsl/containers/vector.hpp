#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/memory/arena.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/type_identity.hpp"
#include "yacsl/ranges/initializer_list.hpp"
#include "yacsl/ranges/integer_range.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/type_traits/allocator.hpp"
#include "yacsl/type_traits/categories.hpp"
#include "yacsl/type_traits/comparaison.hpp"
#include "yacsl/type_traits/convertible_to.hpp"
#include "yacsl/type_traits/range_like.hpp"
#include "yacsl/type_traits/trivial.hpp"

namespace yacsl::containers {

template <typename T, usize N = 0,
          type_traits::AllocatorType Allocator = memory::MainArenaAllocator,
          type_traits::Integral      SizeT     = usize>
class Vector {
public:
  constexpr Vector() noexcept = default;
  constexpr explicit Vector(Allocator const& allocator) noexcept
      : allocator_(allocator) {}

  constexpr Vector(ranges::InitializerList<T> init,
                   Allocator const&           allocator = Allocator());

  template <type_traits::RangeOf<T> Range>
  constexpr Vector(Range const&     range,
                   Allocator const& allocator = Allocator());
  constexpr explicit Vector(SizeT            count,
                            Allocator const& allocator = Allocator());

  constexpr Vector(Vector const& other);
  constexpr Vector(Vector&& other) noexcept;
  constexpr Vector(Vector const&                        other,
                   meta::TypeIdentity<Allocator> const& allocator);
  constexpr Vector(Vector&&                             other,
                   meta::TypeIdentity<Allocator> const& allocator) noexcept;

  constexpr ~Vector() noexcept { free(); }

  constexpr Vector& operator=(this Vector& self YACSL_LIFETIMEBOUND,
                              Vector const&     other);
  constexpr Vector& operator=(this Vector& self YACSL_LIFETIMEBOUND,
                              Vector&&          other) noexcept;

  constexpr Vector& operator=(this Vector& self YACSL_LIFETIMEBOUND,
                              ranges::InitializerList<T> const& init);

  constexpr T& push_back(this Vector& self, T const& value) {
    return self.emplace_back(value);
  }
  constexpr T& push_back(this Vector& self, T&& value) {
    return self.emplace_back(memory::move(value));
  }

  template <typename... Args>
  constexpr T& emplace_back(this Vector& self, Args&&... args);

  constexpr void pop_back(this Vector& self) noexcept;
  constexpr void clear(this Vector& self) noexcept;

  constexpr void reserve(this Vector& self, SizeT capacity);
  constexpr void resize(this Vector& self, SizeT count);

  constexpr void shrink_to_fit(this Vector& self) {
    self.definitely_reserve(self.size_);
  }

  [[nodiscard]] constexpr SizeT size(this Vector const& self) noexcept {
    return self.size_;
  }
  [[nodiscard]] constexpr SizeT capacity(this Vector const& self) noexcept {
    return self.capacity_;
  }

  template <typename Self>
  [[nodiscard]] constexpr type_traits::CopyCV<Self, T>*
  data(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  operator[](this Self&& self YACSL_LIFETIMEBOUND, SizeT index) noexcept {
    return memory::forward<type_traits::CopyCVRef<Self&&, T>>(
        memory::forward<Self>(self).data_[static_cast<usize>(index)]);
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept
      -> ranges::Slice<type_traits::CopyCV<Self, T>, SizeT> {
    return {
        memory::forward<Self>(self).data_,
        memory::forward<Self>(self).size_,
    };
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  reversed_slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept
      -> ranges::ReversedSlice<type_traits::CopyCV<Self, T>, SizeT> {
    return {
        memory::forward<Self>(self).data_,
        memory::forward<Self>(self).size_,
    };
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  bytes(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).slice().bytes();
  }

  [[nodiscard]] constexpr ranges::IntegerRange<SizeT>
  indices(this Vector const& self) noexcept {
    return {0, self.size_};
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  front(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self)[0];
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  back(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self)[self.size_ - 1];
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  begin(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  end(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_ +
           memory::forward<Self>(self).size_;
  }

private:
  void maybe_alloc(this Vector& self, SizeT count); // if size > N

  void free(this Vector& self);

  template <typename Copied> void copy(this Vector& self, Copied const* data);
  void                            move(this Vector& self, T* data);

  void definitely_reserve(this Vector& self, SizeT capacity);

private:
  YACSL_NO_UNIQUE_ADDRESS Allocator allocator_;
  YACSL_NO_UNIQUE_ADDRESS StackArray<T, N, SizeT> fixed_buffer_;

  T*    data_     = fixed_buffer_.data();
  SizeT size_     = 0;
  SizeT capacity_ = fixed_buffer_.size();
};

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(
    ranges::InitializerList<T> init, Allocator const& allocator)
    : allocator_(allocator), size_(static_cast<SizeT>(init.size())) {
  maybe_alloc(size_);
  copy(init.begin());
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
template <type_traits::RangeOf<T> Range>
constexpr Vector<T, N, Allocator, SizeT>::Vector(Range const&     range,
                                                 Allocator const& allocator)
    : allocator_(allocator) {
  if constexpr (requires(Range with_size) {
                  { with_size.size() } -> type_traits::Integral;
                }) {
    size_ = static_cast<SizeT>(range.size());
    maybe_alloc(size_);
    if constexpr (requires(Range with_ptr) {
                    { with_ptr.data() } -> type_traits::OneOf<T*, T const*>;
                  }) {
      copy(range.data());
    } else if constexpr (requires(Range with_ptr) {
                           {
                             with_ptr.begin()
                           } -> type_traits::OneOf<T*, T const*>;
                         }) {
      copy(range.begin());
    } else if constexpr (requires(Range indexable) {
                           { indexable[0] } -> type_traits::ConvertibleTo<T>;
                         }) {
      for (SizeT i = 0; i < size_; ++i) {
        memory::construct_at(data_ + i, range[i]);
      }
    } else {
      auto iter = range.begin();
      for (SizeT i = 0; i < size_; ++iter, ++i) {
        memory::construct_at(data_ + i, *iter);
      }
    }
  } else {
    for (auto&& element : range) {
      emplace_back(memory::forward<decltype(element)>(element));
    }
  }
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(SizeT            count,
                                                 Allocator const& allocator)
    : allocator_(allocator), size_(count) {
  maybe_alloc(size_);
  for (SizeT i = 0; i < count; ++i) {
    memory::construct_at(data_ + i);
  }
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(Vector const& other)
    : allocator_(other.allocator_), size_(other.size_) {
  maybe_alloc(size_);
  copy(other.data_);
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>::Vector(Vector&& other) noexcept
    : allocator_(memory::move(other.allocator_)), size_(other.size_) {
  if constexpr (N > 0) {
    if (other.data_ == other.fixed_buffer_.data()) {
      move(other.data_);
      return;
    }
  }
  data_       = other.data_;
  other.size_ = other.capacity_ = 0;
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(this Vector& self YACSL_LIFETIMEBOUND,
                                          Vector const&     other) {
  if (&other == &self) [[unlikely]] {
    return self;
  }
  if (self.allocator_ == other.allocator_) {
    self.clear();
    self.reserve(other.size_);
  } else {
    self.free();
    self.maybe_alloc(other.size_);
  }
  self.allocator_ = other.allocator_;
  self.size_      = other.size_;
  self.copy(other.data_);
  return self;
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(this Vector& self YACSL_LIFETIMEBOUND,
                                          Vector&&          other) noexcept {

  if (self.allocator_ != other.allocator_) {
    self.free();

    self.allocator_ = memory::move(other.allocator_);

    self.maybe_alloc(other.size_);
    self.move(other.data_);
  } else if (N > 0 && other.data_ == other.fixed_buffer_.data()) {
    self.clear();
    self.move(other.data_);
  } else {
    self.data_ = other.data_;
  }
  self.allocator_ = memory::move(other.allocator_);
  self.size_      = other.size_;
  other.size_ = other.capacity_ = 0;
  return self;
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
constexpr Vector<T, N, Allocator, SizeT>&
Vector<T, N, Allocator, SizeT>::operator=(
    this Vector& self                 YACSL_LIFETIMEBOUND,
    ranges::InitializerList<T> const& init) {
  self.clear();
  self.size_ = static_cast<SizeT>(init.size());
  self.reserve(self.size_);
  self.copy(init.begin());
  return self;
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
template <typename... Args>
constexpr T& Vector<T, N, Allocator, SizeT>::emplace_back(this Vector& self,
                                                          Args&&... args) {
  if (self.size_ == self.capacity_) [[unlikely]] {
    self.definitely_reserve(static_cast<SizeT>(self.capacity_ * 2 + 1));
  }
  return *memory::construct_at(self.data_ + self.size_++,
                               memory::forward<Args>(args)...);
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
constexpr void
Vector<T, N, Allocator, SizeT>::pop_back(this Vector& self) noexcept {
  self.data_[--self.size_].~T();
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
constexpr void
Vector<T, N, Allocator, SizeT>::clear(this Vector& self) noexcept {
  while (self.size_ > 0) {
    self.pop_back();
  }
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::reserve(this Vector& self,
                                                       SizeT        capacity) {
  if (capacity > self.capacity_) {
    self.definitely_reserve(capacity);
  }
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
constexpr void Vector<T, N, Allocator, SizeT>::resize(this Vector& self,
                                                      SizeT        count) {
  if (self.size_ >= count) {
    if constexpr (type_traits::TriviallyDestructible<T>) {
      self.size_ = count;
    } else {
      while (self.size_ > count) {
        self.pop_back();
      }
    }
  } else {
    self.reserve(count);
    for (; self.size_ < count; ++self.size_) {
      memory::construct_at(self.data_ + self.size_);
    }
  }
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
void Vector<T, N, Allocator, SizeT>::maybe_alloc(this Vector& self,
                                                 SizeT        count) {
  if (N == 0 || count > static_cast<SizeT>(N)) {
    self.capacity_ = count;
    self.data_     = reinterpret_cast<T*>(
        self.allocator_
            .allocate(static_cast<Allocator::SizeT>(self.capacity_ * sizeof(T)),
                      memory::align_of<T>)
            .data());
  }
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
void Vector<T, N, Allocator, SizeT>::free(this Vector& self) {
  self.clear();
  if (N == 0 || self.data_ != self.fixed_buffer_.data()) {
    self.allocator_.deallocate({
        reinterpret_cast<uchar*>(self.data_),
        static_cast<Allocator::SizeT>(self.capacity_) * sizeof(T),
    });
  }
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
void Vector<T, N, Allocator, SizeT>::definitely_reserve(this Vector& self,
                                                        SizeT        capacity) {
  T* old_data = self.data_;
  self.data_  = reinterpret_cast<T*>(
      self.allocator_
          .allocate(static_cast<Allocator::SizeT>(capacity) * sizeof(T),
                    memory::align_of<T>)
          .data());
  self.move(old_data);

  for (SizeT i = 0; i < self.size_; ++i) {
    old_data[i].~T();
  }

  if constexpr (N > 0) {
    if (old_data != self.fixed_buffer_.data()) {
      self.allocator_.deallocate({
          reinterpret_cast<uchar*>(old_data),
          static_cast<Allocator::SizeT>(self.capacity_) * sizeof(T),
      });
    }
  } else {
    self.allocator_.deallocate({
        reinterpret_cast<uchar*>(old_data),
        static_cast<Allocator::SizeT>(self.capacity_) * sizeof(T),
    });
  }
  self.capacity_ = capacity;
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
template <typename Copied>
void Vector<T, N, Allocator, SizeT>::copy(this Vector&  self,
                                          Copied const* data) {
  memory::copy({data, static_cast<usize>(self.size_)}, self.data_);
}

template <typename T, usize N, type_traits::AllocatorType Allocator,
          type_traits::Integral SizeT>
void Vector<T, N, Allocator, SizeT>::move(this Vector& self, T* data) {
  if constexpr (type_traits::TriviallyCopyable<T>) {
    if (self.size_ > 0) {
      memory::copy(ranges::Slice{data, static_cast<usize>(self.size_)},
                   self.data_);
    }
  } else {
    for (SizeT i = 0; i < self.size_; ++i) {
      memory::construct_at(self.data_ + i, memory::move(data[i]));
    }
  }
}

} // namespace yacsl::containers

#ifdef YACSL_LSP_ONLY
template class yacsl::containers::Vector<int>; // Instantiates to avoid
                                               // 'arena.hpp is unused'.
#endif
