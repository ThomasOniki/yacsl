#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/containers/stack_array_storage.hpp"
#include "yacsl/memory/alignment.hpp"
#include "yacsl/memory/allocator_type.hpp"
#include "yacsl/memory/arena.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/same_as.hpp"
#include "yacsl/meta/type_identity.hpp"
#include "yacsl/ranges/initializer_list.hpp"
#include "yacsl/ranges/integer_range.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::containers {

template <typename T, usize N = 0,
          memory::AllocatorType A     = memory::MainArenaAllocator,
          meta::Integral        SizeT = usize>
class Vector {
public:
  constexpr Vector() noexcept = default;
  constexpr explicit Vector(A const& allocator) noexcept
      : allocator_(allocator) {}

  constexpr Vector(ranges::InitializerList<T> init, A const& allocator = A());

  template <meta::RangeOf<T> Range>
  constexpr Vector(Range const& range, A const& allocator = A());
  constexpr explicit Vector(SizeT count, A const& allocator = A());

  constexpr Vector(Vector const& other);
  constexpr Vector(Vector&& other) noexcept;
  constexpr Vector(Vector const& other, meta::TypeIdentity<A> const& allocator);
  constexpr Vector(Vector&&                     other,
                   meta::TypeIdentity<A> const& allocator) noexcept;

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
    self.definitely_reserve(self.len_);
  }

  [[nodiscard]] constexpr SizeT len(this Vector const& self) noexcept {
    return self.len_;
  }
  [[nodiscard]] constexpr SizeT capacity(this Vector const& self) noexcept {
    return self.capacity_;
  }

  template <typename Self>
  [[nodiscard]] constexpr meta::CopyCV<Self, T>*
  data(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  operator[](this Self&& self YACSL_LIFETIMEBOUND, SizeT index) noexcept {
    return memory::forward<meta::CopyCVRef<Self&&, T>>(
        memory::forward<Self>(self).data_[static_cast<usize>(index)]);
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept
      -> ranges::Slice<meta::CopyCV<Self, T>, SizeT> {
    return {
        memory::forward<Self>(self).data_,
        memory::forward<Self>(self).len_,
    };
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  reversed_slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept
      -> ranges::ReversedSlice<meta::CopyCV<Self, T>, SizeT> {
    return {
        memory::forward<Self>(self).data_,
        memory::forward<Self>(self).len_,
    };
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  bytes(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).slice().bytes();
  }

  [[nodiscard]] constexpr ranges::IntegerRange<SizeT>
  indices(this Vector const& self) noexcept {
    return {0, self.len_};
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  first(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self)[0];
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  last(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self)[self.len_ - 1];
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  begin(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  end(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_ + memory::forward<Self>(self).len_;
  }

private:
  void maybe_alloc(this Vector& self, SizeT count); // if size > N

  void free(this Vector& self);

  template <typename Copied> void copy(this Vector& self, Copied const* data);
  void                            move(this Vector& self, T* data);

  void definitely_reserve(this Vector& self, SizeT capacity);

private:
  YACSL_NO_UNIQUE_ADDRESS StackArrayStorage<T, N, SizeT> fixed_buffer_ = {};
  YACSL_NO_UNIQUE_ADDRESS A                              allocator_;

  T*    data_     = fixed_buffer_.data();
  SizeT len_      = 0;
  SizeT capacity_ = fixed_buffer_.len();
};

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
constexpr Vector<T, N, A, SizeT>::Vector(ranges::InitializerList<T> init,
                                         A const&                   allocator)
    : allocator_(allocator), len_(static_cast<SizeT>(init.size())) {
  maybe_alloc(len_);
  copy(init.begin());
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
template <meta::RangeOf<T> Range>
constexpr Vector<T, N, A, SizeT>::Vector(Range const& range, A const& allocator)
    : allocator_(allocator) {
  if constexpr (requires(Range with_size) {
                  { with_size.len() } -> meta::Integral;
                }) {
    len_ = static_cast<SizeT>(range.len());
    maybe_alloc(len_);
    if constexpr (requires(Range with_ptr) {
                    { with_ptr.data() } -> meta::OneOf<T*, T const*>;
                  }) {
      copy(range.data());
    } else if constexpr (requires(Range with_ptr) {
                           { with_ptr.begin() } -> meta::OneOf<T*, T const*>;
                         }) {
      copy(range.begin());
    } else if constexpr (requires(Range indexable) {
                           { indexable[0] } -> meta::ConvertibleTo<T>;
                         }) {
      for (SizeT i = 0; i < len_; ++i) {
        memory::construct_at(data_ + i, range[i]);
      }
    } else {
      auto iter = range.begin();
      for (SizeT i = 0; i < len_; ++iter, ++i) {
        memory::construct_at(data_ + i, *iter);
      }
    }
  } else {
    for (auto&& element : range) {
      emplace_back(memory::forward<decltype(element)>(element));
    }
  }
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
constexpr Vector<T, N, A, SizeT>::Vector(SizeT count, A const& allocator)
    : allocator_(allocator), len_(count) {
  maybe_alloc(len_);
  for (SizeT i = 0; i < count; ++i) {
    memory::construct_at(data_ + i);
  }
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
constexpr Vector<T, N, A, SizeT>::Vector(Vector const& other)
    : allocator_(other.allocator_), len_(other.len_) {
  maybe_alloc(len_);
  copy(other.data_);
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
constexpr Vector<T, N, A, SizeT>::Vector(Vector&& other) noexcept
    : allocator_(memory::move(other.allocator_)), len_(other.len_) {
  if constexpr (N > 0) {
    if (other.data_ == other.fixed_buffer_.data()) {
      move(other.data_);
      return;
    }
  }
  data_      = other.data_;
  other.len_ = other.capacity_ = 0;
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
constexpr Vector<T, N, A, SizeT>&
Vector<T, N, A, SizeT>::operator=(this Vector& self YACSL_LIFETIMEBOUND,
                                  Vector const&     other) {
  if (&other == &self) [[unlikely]] {
    return self;
  }
  if (self.allocator_ == other.allocator_) {
    self.clear();
    self.reserve(other.len_);
  } else {
    self.free();
    self.maybe_alloc(other.len_);
  }
  self.allocator_ = other.allocator_;
  self.len_       = other.len_;
  self.copy(other.data_);
  return self;
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
constexpr Vector<T, N, A, SizeT>&
Vector<T, N, A, SizeT>::operator=(this Vector& self YACSL_LIFETIMEBOUND,
                                  Vector&&          other) noexcept {

  if (self.allocator_ != other.allocator_) {
    self.free();

    self.allocator_ = memory::move(other.allocator_);

    self.maybe_alloc(other.len_);
    self.move(other.data_);
  } else if (N > 0 && other.data_ == other.fixed_buffer_.data()) {
    self.clear();
    self.move(other.data_);
  } else {
    self.data_ = other.data_;
  }
  self.allocator_ = memory::move(other.allocator_);
  self.len_       = other.len_;
  other.len_ = other.capacity_ = 0;
  return self;
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
constexpr Vector<T, N, A, SizeT>&
Vector<T, N, A, SizeT>::operator=(this Vector& self YACSL_LIFETIMEBOUND,
                                  ranges::InitializerList<T> const& init) {
  self.clear();
  self.len_ = static_cast<SizeT>(init.size());
  self.reserve(self.len_);
  self.copy(init.begin());
  return self;
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
template <typename... Args>
constexpr T& Vector<T, N, A, SizeT>::emplace_back(this Vector& self,
                                                  Args&&... args) {
  if (self.len_ == self.capacity_) [[unlikely]] {
    self.definitely_reserve(static_cast<SizeT>(self.capacity_ * 2 + 1));
  }
  return *memory::construct_at(self.data_ + self.len_++,
                               memory::forward<Args>(args)...);
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
constexpr void Vector<T, N, A, SizeT>::pop_back(this Vector& self) noexcept {
  self.data_[--self.len_].~T();
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
constexpr void Vector<T, N, A, SizeT>::clear(this Vector& self) noexcept {
  while (self.len_ > 0) {
    self.pop_back();
  }
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
constexpr void Vector<T, N, A, SizeT>::reserve(this Vector& self,
                                               SizeT        capacity) {
  if (capacity > self.capacity_) {
    self.definitely_reserve(capacity);
  }
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
constexpr void Vector<T, N, A, SizeT>::resize(this Vector& self, SizeT count) {
  if (self.len_ >= count) {
    if constexpr (meta::TriviallyDestructible<T>) {
      self.len_ = count;
    } else {
      while (self.len_ > count) {
        self.pop_back();
      }
    }
  } else {
    self.reserve(count);
    for (; self.len_ < count; ++self.len_) {
      memory::construct_at(self.data_ + self.len_);
    }
  }
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
void Vector<T, N, A, SizeT>::maybe_alloc(this Vector& self, SizeT count) {
  if (N == 0 || count > static_cast<SizeT>(N)) {
    self.capacity_ = count;
    self.data_     = reinterpret_cast<T*>(
        self.allocator_
            .allocate(static_cast<A::SizeT>(self.capacity_ * sizeof(T)),
                      memory::align_of<T>)
            .data());
  }
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
void Vector<T, N, A, SizeT>::free(this Vector& self) {
  self.clear();
  if (N == 0 || self.data_ != self.fixed_buffer_.data()) {
    self.allocator_.deallocate({
        reinterpret_cast<u8*>(self.data_),
        static_cast<A::SizeT>(self.capacity_) * sizeof(T),
    });
  }
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
void Vector<T, N, A, SizeT>::definitely_reserve(this Vector& self,
                                                SizeT        capacity) {
  T* old_data = self.data_;
  self.data_  = reinterpret_cast<T*>(
      self.allocator_
          .allocate(static_cast<A::SizeT>(capacity) * sizeof(T),
                    memory::align_of<T>)
          .data());
  self.move(old_data);

  for (SizeT i = 0; i < self.len_; ++i) {
    old_data[i].~T();
  }

  if constexpr (N > 0) {
    if (old_data != self.fixed_buffer_.data()) {
      self.allocator_.deallocate({
          reinterpret_cast<u8*>(old_data),
          static_cast<A::SizeT>(self.capacity_) * sizeof(T),
      });
    }
  } else {
    self.allocator_.deallocate({
        reinterpret_cast<u8*>(old_data),
        static_cast<A::SizeT>(self.capacity_) * sizeof(T),
    });
  }
  self.capacity_ = capacity;
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
template <typename Copied>
void Vector<T, N, A, SizeT>::copy(this Vector& self, Copied const* data) {
  memory::copy({data, static_cast<usize>(self.len_)}, self.data_);
}

template <typename T, usize N, memory::AllocatorType A, meta::Integral SizeT>
void Vector<T, N, A, SizeT>::move(this Vector& self, T* data) {
  if constexpr (meta::TriviallyCopyable<T>) {
    if (self.len_ > 0) {
      memory::copy(ranges::Slice{data, static_cast<usize>(self.len_)},
                   self.data_);
    }
  } else {
    for (SizeT i = 0; i < self.len_; ++i) {
      memory::construct_at(self.data_ + i, memory::move(data[i]));
    }
  }
}

} // namespace yacsl::containers

#ifdef YACSL_LSP_ONLY
template class yacsl::containers::Vector<int>; // Instantiates to avoid
                                               // 'arena.hpp is unused'.
#endif
