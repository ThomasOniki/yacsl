#pragma once

#include "base/attributes.hpp"
#include "base/ranges/integer_range.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename T, Integral SizeT = usize, bool Reversed = false> class Slice {
public:
  friend class Slice<T const, SizeT>;

  constexpr Slice(T* begin BASE_LIFETIMEBOUND, T* end BASE_LIFETIMEBOUND) noexcept
      : data_(Reversed ? end - 1 : begin), size_(static_cast<SizeT>(end - begin)) {}

  constexpr Slice(T* data BASE_LIFETIMEBOUND, SizeT size) noexcept
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

  // The move and forward functions are in lifetimes.hpp which includes this file, so they cannot be
  // used here.
  // NOLINTBEGIN(cppcoreguidelines-missing-std-forward)
  // NOLINTBEGIN(cppcoreguidelines-rvalue-reference-param-not-moved)
  [[nodiscard]] constexpr T*       data(this Slice& self) noexcept { return self.data_; }
  [[nodiscard]] constexpr T*       data(this Slice&& self) noexcept { return self.data_; }
  [[nodiscard]] constexpr T const* data(this Slice const& self) noexcept { return self.data_; }
  [[nodiscard]] constexpr SizeT    size(this Slice const& self) noexcept { return self.size_; }

  [[nodiscard]] constexpr T& operator[](this Slice& self, SizeT index) noexcept {
    return self.data_[Reversed ? -index : index];
  }
  [[nodiscard]] constexpr T&& operator[](this Slice&& self, SizeT index) noexcept {
    return static_cast<T&&>(self.data_[Reversed ? -index : index]);
  }
  [[nodiscard]] constexpr T const& operator[](this Slice const& self, SizeT index) noexcept {
    return self.data_[Reversed ? -index : index];
  }
  [[nodiscard]] constexpr T const&& operator[](this Slice const&& self, SizeT index) noexcept {
    return static_cast<T const&&>(self.data_[Reversed ? -index : index]);
  }

  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes(this Slice& self) noexcept
    requires(!ConstType<T>)
  {
    return {reinterpret_cast<uchar*>(self.data_), static_cast<SizeT>(self.size_ * sizeof(T))};
  }
  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes(this Slice&& self) noexcept
    requires(!ConstType<T>)
  {
    return {reinterpret_cast<uchar*>(self.data_), static_cast<SizeT>(self.size_ * sizeof(T))};
  }
  [[nodiscard]] constexpr Slice<uchar const, SizeT> bytes(this Slice const& self) noexcept {
    return {reinterpret_cast<uchar const*>(self.data_), static_cast<SizeT>(self.size_ * sizeof(T))};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto slice(this Self&& self) noexcept
      -> Slice<RemoveReference<decltype(*self.begin())>, SizeT, false> {
    return {static_cast<Self&&>(self).data_, static_cast<Self&&>(self).size_};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto reversed_slice(this Self&& self) noexcept
      -> Slice<RemoveReference<decltype(*self.begin())>, SizeT, true> {
    return {static_cast<Self&&>(self).data_, static_cast<Self&&>(self).size_};
  }

  [[nodiscard]] constexpr IntegerRange<SizeT> indices(this Slice const& self) noexcept {
    return {0, self.size_};
  }

  [[nodiscard]] constexpr bool operator==(this Slice const& self, Slice const& other) noexcept {
    if (other.size_ != self.size_) {
      return false;
    }
    for (SizeT i = 0; i < self.size_; ++i) {
      if (other.data_[i] != self.data_[i]) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] constexpr T* begin(this Slice& self) noexcept { return self.data_; }
  [[nodiscard]] constexpr T* end(this Slice& self) noexcept { return self.data_ + self.size_; }
  [[nodiscard]] constexpr T* begin(this Slice&& self) noexcept { return self.data_; }
  [[nodiscard]] constexpr T* end(this Slice&& self) noexcept { return self.data_ + self.size_; }
  [[nodiscard]] constexpr T const* begin(this Slice const& self) noexcept { return self.data_; }
  [[nodiscard]] constexpr T const* end(this Slice const& self) noexcept {
    return self.data_ + self.size_;
  }
  // NOLINTEND(cppcoreguidelines-rvalue-reference-param-not-moved)
  // NOLINTEND(cppcoreguidelines-missing-std-forward)

private:
  T*    data_;
  SizeT size_;
};

template <typename T, Integral SizeT = usize> using ReversedSlice = Slice<T, SizeT, true>;

} // namespace base
