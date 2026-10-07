#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/conditional.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/ranges/initializer_list.hpp"
#include "yacsl/ranges/integer_range.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::ranges {

template <typename T, meta::Integral SizeT = usize, bool Reversed = false>
class Slice {
public:
  friend class Slice<T const, SizeT>;

  constexpr Slice(T* begin YACSL_LIFETIMEBOUND,
                  T* end   YACSL_LIFETIMEBOUND) noexcept
      : data_(Reversed ? end - 1 : begin),
        size_(static_cast<SizeT>(end - begin)) {}

  constexpr Slice(T* data YACSL_LIFETIMEBOUND, SizeT size) noexcept
      : data_(Reversed ? data + size - 1 : data), size_(size) {}

  constexpr Slice(InitializerList<T> data YACSL_LIFETIMEBOUND) noexcept
    requires meta::ConstType<T>
      : data_(data.begin()), size_(data.size()) {}

  constexpr Slice(Slice<T, SizeT, !Reversed> other) noexcept
      : data_(Reversed ? other.end() - 1 : other.end() + 1),
        size_(other.size() + 1) {}

  // If you just use RemoveConst<T> without the Conditional and the requires
  // ConstType<T> clause is not satisfied then it deletes the copy constructor
  // instead of not defining it.
  constexpr Slice(
      Slice<meta::Conditional<meta::ConstType<T>, meta::RemoveConst<T>, void>,
            SizeT> const& mutable_slice) noexcept
    requires meta::ConstType<T>
      : data_(mutable_slice.data_), size_(mutable_slice.size_) {}

  constexpr Slice& operator=(
      Slice<meta::Conditional<meta::ConstType<T>, meta::RemoveConst<T>, void>,
            SizeT> const& mutable_slice) noexcept
    requires meta::ConstType<T>
  {
    data_ = mutable_slice.data_;
    size_ = mutable_slice.size_;
    return *this;
  }

  // The memory::move and memory::forward functions are in lifetimes.hpp which
  // includes this file, so they cannot be used here.
  // NOLINTBEGIN(cppcoreguidelines-missing-std-forward)
  // NOLINTBEGIN(cppcoreguidelines-rvalue-reference-param-not-moved)
  template <typename Self>
  [[nodiscard]] constexpr meta::CopyCV<Self, T>*
  data(this Self&& self) noexcept {
    return self.data_;
  }
  [[nodiscard]] constexpr SizeT size(this Slice const& self) noexcept {
    return self.size_;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&& operator[](this Self&& self,
                                            SizeT       index) noexcept {
    return static_cast<meta::CopyCVRef<Self&&, T>>(
        self.data_[Reversed ? -index : index]);
  }

  template <typename Self>
  [[nodiscard]] constexpr auto bytes(this Self&& self) noexcept {
    if constexpr (meta::ConstType<T>) {
      using Byte = meta::CopyCV<Self, u8 const>;
      return Slice<Byte, SizeT>{
          reinterpret_cast<Byte*>(self.data_),
          static_cast<SizeT>(self.size_ * sizeof(T)),
      };
    } else {
      using Byte = meta::CopyCV<Self, u8>;
      return Slice<Byte, SizeT>{
          reinterpret_cast<Byte*>(self.data_),
          static_cast<SizeT>(self.size_ * sizeof(T)),
      };
    }
  }

  template <typename Self>
  [[nodiscard]] constexpr auto slice(this Self&& self) noexcept
      -> Slice<meta::CopyCV<Self, T>, SizeT, false> {
    return {static_cast<Self&&>(self).data_, static_cast<Self&&>(self).size_};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto reversed_slice(this Self&& self) noexcept
      -> Slice<meta::CopyCV<Self, T>, SizeT, true> {
    return {static_cast<Self&&>(self).data_, static_cast<Self&&>(self).size_};
  }

  [[nodiscard]] constexpr IntegerRange<SizeT>
  indices(this Slice const& self) noexcept {
    return {0, self.size_};
  }

  [[nodiscard]] constexpr bool operator==(this Slice const& self,
                                          Slice const&      other) noexcept {
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

  template <typename Self>
  [[nodiscard]] constexpr meta::CopyCV<Self, T>*
  begin(this Self&& self) noexcept {
    return self.data_;
  }
  template <typename Self>
  [[nodiscard]] constexpr meta::CopyCV<Self, T>*
  end(this Self&& self) noexcept {
    return self.data_ + self.size_;
  }
  // NOLINTEND(cppcoreguidelines-rvalue-reference-param-not-moved)
  // NOLINTEND(cppcoreguidelines-missing-std-forward)

private:
  T*    data_;
  SizeT size_;
};

template <typename T, meta::Integral SizeT = usize>
using ReversedSlice = Slice<T, SizeT, true>;

} // namespace yacsl::ranges
