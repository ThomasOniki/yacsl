#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/type_traits/categories.hpp"
#include "yacsl/type_traits/qualifiers.hpp"

namespace yacsl::containers {

template <usize N, type_traits::Character C = char,
          type_traits::Integral SizeT = usize>
class NTTPString {
public:
  constexpr NTTPString(C const (&str)[N + 1]) noexcept {
    for (usize i = 0; i < N; ++i) {
      data_[i] = str[i];
    }
  }

  constexpr NTTPString(ranges::BasicStringView<C, SizeT> view) noexcept {
    for (SizeT i = 0; i < view.size(); ++i) {
      data_[i] = view[i];
    }
  }

  constexpr NTTPString() noexcept = default;

  [[nodiscard]] constexpr SizeT size(this const NTTPString& /*self*/) noexcept {
    return static_cast<SizeT>(N);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  data(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.data();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  operator[](this Self&& self YACSL_LIFETIMEBOUND, SizeT index) noexcept {
    return memory::forward<Self>(self).data_[index];
  }

  template <typename Self>
  [[nodiscard]] constexpr ranges::BasicStringView<C, SizeT>
  view(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return {memory::forward<Self>(self).data_.data(), static_cast<SizeT>(N)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.slice();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  reversed_slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.reversed_slice();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  bytes(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.bytes();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  begin(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.begin();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  end(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.end();
  }

public:
  StackArray<C, N, SizeT> data_ = {'\0'};
};

template <type_traits::Character C, type_traits::Integral SizeT>
class NTTPString<0, C, SizeT> {
public:
  constexpr NTTPString(C const (& /*str*/)[1]) noexcept {}
  constexpr NTTPString() noexcept = default;

  [[nodiscard]] constexpr SizeT size(this NTTPString const& /*self*/) noexcept {
    return 0;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  data(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    auto&& ref = memory::forward<Self>(self).char_;
    return &ref;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  operator[](this Self&& self YACSL_LIFETIMEBOUND, usize /*index*/) noexcept {
    return memory::forward<Self>(self).char_;
  }

  [[nodiscard]] constexpr ranges::BasicStringView<C, SizeT>
  view(this NTTPString const& self YACSL_LIFETIMEBOUND) noexcept {
    return {&self.char_, static_cast<SizeT>(0)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept
      -> ranges::Slice<type_traits::CopyCV<Self, C>, SizeT> {
    return {memory::forward<Self>(self).data(), static_cast<SizeT>(0)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  reversed_slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept
      -> ranges::ReversedSlice<type_traits::CopyCV<Self, C>, SizeT> {
    return {memory::forward<Self>(self).data(), static_cast<SizeT>(0)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  bytes(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).slice().bytes();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  begin(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  end(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data();
  }

  C char_ = '\0';
};

template <type_traits::Character C, usize N>
NTTPString(C const (&str)[N]) -> NTTPString<N - 1, C, usize>;

template <NTTPString... Strings>
constexpr NTTPString cat_nttpstrings = []() {
  NTTPString<(... + Strings.size())> ret;
  usize                              offset = 0;
  (..., (offset += memory::copy(Strings.slice(), ret.data() + offset).size()));
  return ret;
}();

} // namespace yacsl::containers
