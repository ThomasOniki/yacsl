#pragma once

#include "base/attributes.hpp"
#include "base/containers/stack_array.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <usize N, Character C = char, Integral SizeT = usize> class NTTPString {
public:
  constexpr NTTPString(C const (&str)[N + 1]) noexcept {
    for (usize i = 0; i < N; ++i) {
      data_[i] = str[i];
    }
  }

  constexpr NTTPString(BasicStringView<C, SizeT> view) noexcept {
    for (SizeT i = 0; i < view.size(); ++i) {
      data_[i] = view[i];
    }
  }

  constexpr NTTPString() noexcept = default;

  [[nodiscard]] constexpr SizeT size(this const NTTPString& /*self*/) noexcept {
    return static_cast<SizeT>(N);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto data(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_.data();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& operator[](this Self&& self BASE_LIFETIMEBOUND,
                                            SizeT            index) noexcept {
    return base::forward<Self>(self).data_[index];
  }

  template <typename Self>
  [[nodiscard]] constexpr BasicStringView<C, SizeT>
  view(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return {base::forward<Self>(self).data_.data(), static_cast<SizeT>(N)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto slice(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_.slice();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto reversed_slice(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_.reversed_slice();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto bytes(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_.bytes();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto begin(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_.begin();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto end(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data_.end();
  }

public:
  StackArray<C, N, SizeT> data_ = {'\0'};
};

template <Character C, Integral SizeT> class NTTPString<0, C, SizeT> {
public:
  constexpr NTTPString(C const (& /*str*/)[1]) noexcept {}
  constexpr NTTPString() noexcept = default;

  [[nodiscard]] constexpr SizeT size(this NTTPString const& /*self*/) noexcept { return 0; }
  template <typename Self>
  [[nodiscard]] constexpr auto data(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    auto&& ref = base::forward<Self>(self).char_;
    return &ref;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&& operator[](this Self&& self BASE_LIFETIMEBOUND,
                                            usize /*index*/) noexcept {
    return base::forward<Self>(self).char_;
  }

  [[nodiscard]] constexpr BasicStringView<C, SizeT>
  view(this NTTPString const& self BASE_LIFETIMEBOUND) noexcept {
    return {&self.char_, static_cast<SizeT>(0)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto slice(this Self&& self BASE_LIFETIMEBOUND) noexcept
      -> Slice<RemoveReference<decltype(*self.begin())>, SizeT> {
    return {base::forward<Self>(self).data(), static_cast<SizeT>(0)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto reversed_slice(this Self&& self BASE_LIFETIMEBOUND) noexcept
      -> ReversedSlice<RemoveReference<decltype(*self.begin())>, SizeT> {
    return {base::forward<Self>(self).data(), static_cast<SizeT>(0)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto bytes(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).slice().bytes();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto begin(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto end(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).data();
  }

  C char_ = '\0';
};

template <Character C, usize N> NTTPString(C const (&str)[N]) -> NTTPString<N - 1, C, usize>;

template <NTTPString... Strings>
constexpr NTTPString cat_nttpstrings = []() {
  NTTPString<(... + Strings.size())> ret;
  usize                              offset = 0;
  (..., (offset += copy(Strings.slice(), ret.data() + offset).size()));
  return ret;
}();

} // namespace base
