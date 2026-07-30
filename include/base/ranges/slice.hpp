#pragma once

#include "base/ranges/initializer_list.hpp"
#include "base/ranges/integer_range.hpp"
#include "base/ranges/reverse_iterator.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/comparaison.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename T, Integral SizeT = usize> class Slice {
public:
  friend class Slice<const T, SizeT>;

  constexpr Slice(T* begin, T* end) noexcept
      : data_(begin), size_(static_cast<SizeT>(end - begin)) {}

  constexpr Slice(T* data, SizeT size) noexcept : data_(data), size_(size) {}

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Slice(const Slice<RemoveConst<T>, SizeT>& mutable_slice) noexcept
      : data_(mutable_slice.data_), size_(mutable_slice.size_) {}
  constexpr Slice& operator=(const Slice<RemoveConst<T>, SizeT>& mutable_slice) noexcept {
    data_ = mutable_slice.data_;
    size_ = mutable_slice.size_;
    return *this;
  }

  [[nodiscard]] constexpr T*       data() noexcept { return data_; }
  [[nodiscard]] constexpr const T* data() const noexcept { return data_; }
  [[nodiscard]] constexpr SizeT    size() const noexcept { return size_; }

  [[nodiscard]] constexpr T&       operator[](SizeT index) noexcept { return data_[index]; }
  [[nodiscard]] constexpr const T& operator[](SizeT index) const noexcept { return data_[index]; }

  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept
    requires(!SameAs<T, const T>)
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<uchar*>(data_), static_cast<SizeT>(size_ * sizeof(T))};
  }
  [[nodiscard]] constexpr Slice<const uchar, SizeT> bytes() const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const uchar*>(data_), static_cast<SizeT>(size_ * sizeof(T))};
  }

  [[nodiscard]] constexpr IntegerRange<SizeT> indices() const noexcept { return {0, size_}; }

  [[nodiscard]] constexpr bool operator==(const Slice& other) const noexcept {
    if (other.size_ != size_) {
      return false;
    }
    for (SizeT i = 0; i < size_; ++i) {
      if (other.data_[i] != data_[i]) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] constexpr T*       begin() noexcept { return data_; }
  [[nodiscard]] constexpr T*       end() noexcept { return data_ + size_; }
  [[nodiscard]] constexpr const T* begin() const noexcept { return data_; }
  [[nodiscard]] constexpr const T* end() const noexcept { return data_ + size_; }
  [[nodiscard]] constexpr const T* cbegin() const noexcept { return data_; }
  [[nodiscard]] constexpr const T* cend() const noexcept { return data_ + size_; }

  [[nodiscard]] constexpr ReverseIterator<T*>       rbegin() noexcept { return data_ + size_; }
  [[nodiscard]] constexpr ReverseIterator<T*>       rend() noexcept { return data_; }
  [[nodiscard]] constexpr ReverseIterator<const T*> rbegin() const noexcept {
    return data_ + size_;
  }
  [[nodiscard]] constexpr ReverseIterator<const T*> rend() const noexcept { return data_; }
  [[nodiscard]] constexpr ReverseIterator<const T*> crbegin() const noexcept {
    return data_ + size_;
  }
  [[nodiscard]] constexpr ReverseIterator<const T*> crend() const noexcept { return data_; }

private:
  T*    data_;
  SizeT size_;
};

} // namespace base
