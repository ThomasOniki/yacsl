#pragma once

#include "base/attributes.hpp"
#include "base/ranges/initializer_list.hpp"
#include "base/ranges/integer_range.hpp"
#include "base/type_aliases.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"

namespace base::ranges {

template <typename T, type_traits::Integral SizeT = usize, bool Reversed = false>
class Slice {
public:
  friend class Slice<T const, SizeT>;

  constexpr Slice(T* begin BASE_LIFETIMEBOUND,
                  T* end   BASE_LIFETIMEBOUND) noexcept
      : data_(Reversed ? end - 1 : begin),
        size_(static_cast<SizeT>(end - begin)) {}

  constexpr Slice(T* data BASE_LIFETIMEBOUND, SizeT size) noexcept
      : data_(Reversed ? data + size - 1 : data), size_(size) {}

  constexpr Slice(InitializerList<T> data BASE_LIFETIMEBOUND) noexcept
    requires type_traits::ConstType<T>
      : data_(data.begin()), size_(data.size()) {}

  constexpr Slice(Slice<T, SizeT, !Reversed> other) noexcept
      : data_(Reversed ? other.end() - 1 : other.end() + 1),
        size_(other.size() + 1) {}

  constexpr Slice(Slice<type_traits::RemoveConst<T>, SizeT> const& mutable_slice) noexcept
      : data_(mutable_slice.data_), size_(mutable_slice.size_) {}
  // NOLINTNEXTLINE(bugprone-unhandled-self-assignment)
  constexpr Slice&
  operator=(Slice<type_traits::RemoveConst<T>, SizeT> const& mutable_slice) noexcept {
    data_ = mutable_slice.data_;
    size_ = mutable_slice.size_;
    return *this;
  }

  // The memory::move and memory::forward functions are in lifetimes.hpp which includes this
  // file, so they cannot be used here.
  // NOLINTBEGIN(cppcoreguidelines-missing-std-memory::forward)
  // NOLINTBEGIN(cppcoreguidelines-rvalue-reference-param-not-memory::moved)
  template <typename Self>
  [[nodiscard]] constexpr type_traits::CopyCV<Self, T>* data(this Self&& self) noexcept {
    return self.data_;
  }
  [[nodiscard]] constexpr SizeT size(this Slice const& self) noexcept {
    return self.size_;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&& operator[](this Self&& self,
                                            SizeT       index) noexcept {
    return static_cast<type_traits::CopyCVRef<Self&&, T>>(
        self.data_[Reversed ? -index : index]);
  }

  template <typename Self>
  [[nodiscard]] constexpr auto bytes(this Self&& self) noexcept {
    if constexpr (type_traits::ConstType<T>) {
      using Byte = type_traits::CopyCV<Self, uchar const>;
      return Slice<Byte, SizeT>{
          reinterpret_cast<Byte*>(self.data_),
          static_cast<SizeT>(self.size_ * sizeof(T)),
      };
    } else {
      using Byte = type_traits::CopyCV<Self, uchar>;
      return Slice<Byte, SizeT>{
          reinterpret_cast<Byte*>(self.data_),
          static_cast<SizeT>(self.size_ * sizeof(T)),
      };
    }
  }

  template <typename Self>
  [[nodiscard]] constexpr auto slice(this Self&& self) noexcept
      -> Slice<type_traits::CopyCV<Self, T>, SizeT, false> {
    return {static_cast<Self&&>(self).data_, static_cast<Self&&>(self).size_};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto reversed_slice(this Self&& self) noexcept
      -> Slice<type_traits::CopyCV<Self, T>, SizeT, true> {
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
  [[nodiscard]] constexpr type_traits::CopyCV<Self, T>* begin(this Self&& self) noexcept {
    return self.data_;
  }
  template <typename Self>
  [[nodiscard]] constexpr type_traits::CopyCV<Self, T>* end(this Self&& self) noexcept {
    return self.data_ + self.size_;
  }
  // NOLINTEND(cppcoreguidelines-rvalue-reference-param-not-memory::moved)
  // NOLINTEND(cppcoreguidelines-missing-std-memory::forward)

private:
  T*    data_;
  SizeT size_;
};

template <typename T, type_traits::Integral SizeT = usize>
using ReversedSlice = Slice<T, SizeT, true>;

} // namespace base::ranges
