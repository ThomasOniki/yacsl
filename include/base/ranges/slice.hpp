#pragma once

#include "base/ranges/integer_range.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/comparaison.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename T, Integral SizeT = usize, bool Reversed = false> class Slice {
public:
  friend class Slice<T const, SizeT>;

  constexpr Slice(T* begin, T* end) noexcept
      : data_(Reversed ? end - 1 : begin), size_(static_cast<SizeT>(end - begin)) {}

  constexpr Slice(T* data, SizeT size) noexcept
      : data_(Reversed ? data + size - 1 : data), size_(size) {}

  constexpr Slice(Slice<T, SizeT, !Reversed> other) noexcept
      : data_(Reversed ? other.end() - 1 : other.end() + 1), size_(other.size() + 1) {}

  constexpr Slice(Slice<RemoveConst<T>, SizeT> const& mutable_slice) noexcept
      : data_(mutable_slice.data_), size_(mutable_slice.size_) {}
  // NOLINTNEXTLINE(bugprone-unhandled-self-assignment)
  constexpr Slice& operator=(Slice<RemoveConst<T>, SizeT> const& mutable_slice) noexcept {
    data_ = mutable_slice.data_;
    size_ = mutable_slice.size_;
    return *this;
  }

  [[nodiscard]] constexpr T*       data() noexcept { return data_; }
  [[nodiscard]] constexpr T const* data() const noexcept { return data_; }
  [[nodiscard]] constexpr SizeT    size() const noexcept { return size_; }

  [[nodiscard]] constexpr T& operator[](SizeT index) noexcept {
    return data_[Reversed ? -index : index];
  }
  [[nodiscard]] constexpr T const& operator[](SizeT index) const noexcept {
    return data_[Reversed ? -index : index];
  }

  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept
    requires(!SameAs<T, T const>)
  {
    return {reinterpret_cast<uchar*>(data_), static_cast<SizeT>(size_ * sizeof(T))};
  }
  [[nodiscard]] constexpr Slice<uchar const, SizeT> bytes() const noexcept {
    return {reinterpret_cast<uchar const*>(data_), static_cast<SizeT>(size_ * sizeof(T))};
  }
  [[nodiscard]] constexpr Slice<T, SizeT, false> slice() noexcept { return {begin(), size_}; }
  [[nodiscard]] constexpr Slice<T, SizeT, true>  reversed_slice() noexcept {
    return {begin(), size_};
  }

  [[nodiscard]] constexpr IntegerRange<SizeT> indices() const noexcept { return {0, size_}; }

  [[nodiscard]] constexpr bool operator==(Slice const& other) const noexcept {
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
  [[nodiscard]] constexpr T const* begin() const noexcept { return data_; }
  [[nodiscard]] constexpr T const* end() const noexcept { return data_ + size_; }

private:
  T*    data_;
  SizeT size_;
};

template <typename T, Integral SizeT = usize> using ReversedSlice = Slice<T, SizeT, true>;

} // namespace base
