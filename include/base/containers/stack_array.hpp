#pragma once

#include "base/attributes.hpp"
#include "base/containers/structured_bindings.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/meta/type_manip.hpp"
#include "base/ranges/integer_range.hpp"
#include "base/ranges/slice.hpp"
#include "base/type_aliases.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/limits.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utils/implicit_cast.hpp"

namespace base::containers {

template <typename T, usize N,
          type_traits::Integral SizeT =
              meta::ConditionalT<(N > type_traits::Limits<u32>::MAX), usize, u32>>
class StackArray {
public:
  [[nodiscard]] constexpr SizeT size(this StackArray const& /*self*/) noexcept {
    return static_cast<SizeT>(N);
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  data(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& operator[](this Self&& self BASE_LIFETIMEBOUND,
                                            SizeT            index) noexcept {
    return memory::forward<Self>(self).data_[index];
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  slice(this Self&& self BASE_LIFETIMEBOUND) noexcept
      -> ranges::Slice<type_traits::CopyCV<Self, T>, SizeT> {
    return {memory::forward<Self>(self).data_, static_cast<SizeT>(N)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  reversed_slice(this Self&& self BASE_LIFETIMEBOUND) noexcept
      -> ranges::ReversedSlice<type_traits::CopyCV<Self, T>, SizeT> {
    return {memory::forward<Self>(self).data_, static_cast<SizeT>(N)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  bytes(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).slice().bytes();
  }

  [[nodiscard]] constexpr ranges::IntegerRange<SizeT>
  indices(this StackArray const& /*self*/) noexcept {
    return {0, N};
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  front(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_[0];
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  back(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_[N - 1];
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  begin(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  end(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_ + N;
  }

  template <usize I, typename Self>
  [[nodiscard]] constexpr auto&&
  get(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_[I];
  }

  constexpr bool operator==(this StackArray const& self,
                            StackArray const&      other) noexcept = default;

public:
  T data_[N];
};

template <typename T, type_traits::Integral SizeT>
class StackArray<T, 0, SizeT> {
public:
  [[nodiscard]] constexpr SizeT size(this StackArray const& /*self*/) noexcept {
    return 0;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto data(this Self&& /*self*/) noexcept {
    return utils::implicit_cast<type_traits::CopyCV<Self, T>*>(nullptr);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& operator[](this Self&& /*self*/,
                                            SizeT /*index*/) noexcept {
    return utils::implicit_cast<type_traits::CopyCV<Self, T>*>(nullptr)[0];
  }

  template <typename Self>
  [[nodiscard]] constexpr auto slice(this Self&& /*self*/) noexcept {
    return ranges::Slice<type_traits::CopyCV<Self, T>, SizeT>{nullptr, 0};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto reversed_slice(this Self&& /*self*/) noexcept {
    return ranges::ReversedSlice<type_traits::CopyCV<Self, T>, SizeT>{nullptr,
                                                                      0};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto bytes(this Self&& /*self*/) noexcept {
    return ranges::ReversedSlice<type_traits::CopyCV<Self, uchar>, SizeT>{
        nullptr, 0};
  }

  [[nodiscard]] constexpr ranges::IntegerRange<SizeT>
  indices(this StackArray const& /*self*/) noexcept {
    return {0, 0};
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&& front(this Self&& /*self*/) noexcept {
    return utils::implicit_cast<type_traits::CopyCV<Self, T>*>(nullptr)[0];
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& back(this Self&& /*self*/) noexcept {
    return utils::implicit_cast<type_traits::CopyCV<Self, T>*>(nullptr)[0];
  }

  template <typename Self>
  [[nodiscard]] constexpr auto begin(this Self&& /*self*/) noexcept {
    return utils::implicit_cast<type_traits::CopyCV<Self, T>*>(nullptr);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto end(this Self&& /*self */) noexcept {
    return utils::implicit_cast<type_traits::CopyCV<Self, T>*>(nullptr);
  }
  template <usize I, typename Self>
  [[nodiscard]] constexpr auto&& get(this Self&& /*self*/) noexcept {
    return utils::implicit_cast<type_traits::CopyCV<Self, T>*>(nullptr)[0];
  }

  constexpr bool operator==(this StackArray const& self,
                            StackArray const&      other) noexcept = default;
};

template <typename T, usize N, type_traits::Integral SizeT>
struct TupleSize<StackArray<T, N, SizeT>> {
  static constexpr usize value = N;
};

template <typename T, usize N, type_traits::Integral SizeT, usize I>
struct TupleElement<StackArray<T, N, SizeT>, I> {
  using Type = T;
};

} // namespace base::containers
