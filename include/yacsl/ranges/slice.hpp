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
      : data_(Reversed ? end : begin), len_(static_cast<SizeT>(end - begin)) {}

  constexpr Slice(T* data YACSL_LIFETIMEBOUND, SizeT size) noexcept
      : data_(Reversed ? data + size : data), len_(size) {}

  constexpr Slice(
      InitializerList<meta::RemoveConst<T>> data YACSL_LIFETIMEBOUND) noexcept
    requires meta::ConstType<T>
      : data_(Reversed ? data.end() : data.begin()), len_(data.size()) {}

  constexpr Slice(Slice<T, SizeT, !Reversed> other) noexcept
      : data_(other.end()), len_(other.len()) {}

  // If you just use RemoveConst<T> without the Conditional and the requires
  // ConstType<T> clause is not satisfied then it deletes the copy constructor
  // instead of not defining it.
  constexpr Slice(
      Slice<meta::Conditional<meta::ConstType<T>, meta::RemoveConst<T>, void>,
            SizeT>
          mutable_slice) noexcept
    requires meta::ConstType<T>
      : data_(mutable_slice.data_), len_(mutable_slice.len_) {}

  constexpr Slice& operator=(
      Slice<meta::Conditional<meta::ConstType<T>, meta::RemoveConst<T>, void>,
            SizeT>
          mutable_slice) noexcept
    requires meta::ConstType<T>
  {
    data_ = mutable_slice.data_;
    len_  = mutable_slice.len_;
    return *this;
  }

  // The memory::move and memory::forward functions are in lifetimes.hpp which
  // includes this file, so they cannot be used here.
  // NOLINTBEGIN(cppcoreguidelines-missing-std-forward)
  // NOLINTBEGIN(cppcoreguidelines-rvalue-reference-param-not-moved)
  template <typename Self>
  [[nodiscard]] constexpr meta::CopyCV<Self, T>* data(this Self&& self) noexcept
    requires(!Reversed)
  {
    return self.data_;
  }
  [[nodiscard]] constexpr SizeT len(this Slice self) noexcept {
    return self.len_;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&& operator[](this Self&& self,
                                            SizeT       index) noexcept {
    if (Reversed) {
      return static_cast<meta::CopyCVRef<Self&&, T>>(
          self.data_[static_cast<ptrdiff>(-index - 1)]);
    }
    return static_cast<meta::CopyCVRef<Self&&, T>>(self.data_[index]);
  }

  template <typename Self>
  [[nodiscard]] constexpr auto bytes(this Self&& self) noexcept {
    if constexpr (meta::ConstType<T>) {
      using Byte = meta::CopyCV<Self, u8 const>;
      return Slice<Byte, SizeT>{
          reinterpret_cast<Byte*>(self.data_),
          static_cast<SizeT>(self.len_ * sizeof(T)),
      };
    } else {
      using Byte = meta::CopyCV<Self, u8>;
      return Slice<Byte, SizeT>{
          reinterpret_cast<Byte*>(self.data_),
          static_cast<SizeT>(self.len_ * sizeof(T)),
      };
    }
  }

  template <typename Self>
  [[nodiscard]] constexpr auto slice(this Self&& self) noexcept
      -> Slice<meta::CopyCV<Self, T>, SizeT, false> {
    return {static_cast<Self&&>(self).data_, static_cast<Self&&>(self).len_};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto reversed_slice(this Self&& self) noexcept
      -> Slice<meta::CopyCV<Self, T>, SizeT, true> {
    return {static_cast<Self&&>(self).data_, static_cast<Self&&>(self).len_};
  }

  [[nodiscard]] constexpr IntegerRange<SizeT>
  indices(this Slice self) noexcept {
    return {0, self.len_};
  }

  [[nodiscard]] constexpr bool operator==(this Slice self,
                                          Slice      other) noexcept {
    if (other.len_ != self.len_) {
      return false;
    }
    for (SizeT i = 0; i < self.len_; ++i) {
      if (other.data_[i] != self.data_[i]) {
        return false;
      }
    }
    return true;
  }

  template <typename Self>
  [[nodiscard]] constexpr meta::CopyCV<Self, T>*
  begin(this Self&& self) noexcept
    requires(!Reversed)
  {
    return self.data_;
  }
  template <typename Self>
  [[nodiscard]] constexpr meta::CopyCV<Self, T>* end(this Self&& self) noexcept
    requires(!Reversed)
  {
    return self.data_ + self.len_;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto begin(this Self&& self) noexcept
    requires Reversed
  {
    return ReverseIterator<meta::CopyCVRef<Self&&, T>>(self.data_ + self.len_);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto end(this Self&& self) noexcept
    requires Reversed
  {
    return ReverseIterator<meta::CopyCVRef<Self&&, T>>(self.data_);
  }

private:
  template <typename Ref> class ReverseIterator {
  public:
    explicit constexpr ReverseIterator(T* ptr) noexcept : ptr_(ptr) {}

    constexpr bool operator==(this ReverseIterator self,
                              ReverseIterator      other) noexcept = default;

    constexpr auto&& operator*(this ReverseIterator self) noexcept {
      return static_cast<Ref>(self.ptr_[-1]);
    }

    template <typename Self>
    constexpr auto&& operator++(this Self&& self) noexcept {
      --self.ptr_;
      return static_cast<Self&&>(self);
    }
    template <meta::MutableType Self>
    constexpr auto operator++(this Self&& self,
                              int /*postfix placeholder*/) noexcept {
      return ReverseIterator(self.ptr_--);
    }

    template <meta::MutableType Self>
    constexpr auto&& operator--(this Self&& self) noexcept {
      ++self.ptr_;
      return static_cast<Self&&>(self);
    }
    template <meta::MutableType Self>
    constexpr auto operator--(this Self&& self,
                              int /*postfix placeholder*/) noexcept {
      return ReverseIterator(self.ptr_++);
    }

  private:
    T* ptr_;
  };

private:
  T*    data_;
  SizeT len_;
};

// NOLINTEND(cppcoreguidelines-rvalue-reference-param-not-moved)
// NOLINTEND(cppcoreguidelines-missing-std-forward)

template <typename T, meta::Integral SizeT = usize>
using ReversedSlice = Slice<T, SizeT, true>;

} // namespace yacsl::ranges
