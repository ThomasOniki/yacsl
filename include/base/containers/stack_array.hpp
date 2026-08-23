#pragma once

#include "base/attributes.hpp"
#include "base/containers/structured_bindings.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/metaprogramming/type_manip.hpp"
#include "base/ranges/integer_range.hpp"
#include "base/ranges/slice.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/limits.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
template <typename T, usize N, Integral SizeT = ConditionalT<(N > Limits<u32>::max), usize, u32>>
class StackArray {
public:
  [[nodiscard]] constexpr SizeT size(this StackArray const& /*self*/) noexcept {
    return static_cast<SizeT>(N);
  }

  template <typename Self>
  [[nodiscard]] constexpr auto data(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& operator[](this Self&& self BASE_LIFETIMEBOUND,
                                            SizeT            index) noexcept {
    return base::forward<Self>(self).data_[index];
  }

  template <typename Self>
  [[nodiscard]] constexpr auto slice(this Self&& self BASE_LIFETIMEBOUND) noexcept
      -> Slice<RemoveReference<decltype(*self.begin())>, SizeT> {
    return {base::forward<Self>(self).data_, static_cast<SizeT>(N)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto reversed_slice(this Self&& self BASE_LIFETIMEBOUND) noexcept
      -> ReversedSlice<RemoveReference<decltype(*self.begin())>, SizeT> {
    return {base::forward<Self>(self).data_, static_cast<SizeT>(N)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto bytes(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).slice().bytes();
  }

  [[nodiscard]] constexpr IntegerRange<SizeT> indices(this StackArray const& /*self*/) noexcept {
    return {0, N};
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&& front(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_[0];
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& back(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_[N - 1];
  }

  template <typename Self>
  [[nodiscard]] constexpr auto begin(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto end(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_ + N;
  }

  template <usize I, typename Self>
  [[nodiscard]] constexpr auto&& get(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_[I];
  }

  constexpr bool operator==(this StackArray const& self,
                            StackArray const&      other) noexcept = default;

public:
  T data_[N]; // NOLINT
};
// NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)

template <typename T, usize N, Integral SizeT> struct TupleSize<StackArray<T, N, SizeT>> {
  static constexpr usize value = N;
};

template <typename T, usize N, Integral SizeT, usize I>
struct TupleElement<StackArray<T, N, SizeT>, I> {
  using Type = T;
};

} // namespace base
