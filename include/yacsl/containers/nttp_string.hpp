#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::containers {

template <usize N, meta::Character T = char, meta::Integral SizeT = usize>
class NTTPString {
public:
  constexpr NTTPString(T const (&str)[N + 1]) noexcept {
    for (usize i = 0; i < N; ++i) {
      data_[i] = str[i];
    }
  }

  constexpr NTTPString(ranges::GenericStringView<T, SizeT> view) noexcept {
    for (SizeT i = 0; i < view.len(); ++i) {
      data_[i] = view[i];
    }
  }

  constexpr NTTPString() noexcept = default;

  [[nodiscard]] constexpr SizeT len(this const NTTPString& /*self*/) noexcept {
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
  [[nodiscard]] constexpr ranges::GenericStringView<T, SizeT>
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
  StackArray<T, N, SizeT> data_ = {'\0'};
};

template <meta::Character T, meta::Integral SizeT>
class NTTPString<0, T, SizeT> {
public:
  constexpr NTTPString(T const (& /*str*/)[1]) noexcept {}
  constexpr NTTPString() noexcept = default;

  [[nodiscard]] constexpr SizeT len(this NTTPString const& /*self*/) noexcept {
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

  [[nodiscard]] constexpr ranges::GenericStringView<T, SizeT>
  view(this NTTPString const& self YACSL_LIFETIMEBOUND) noexcept {
    return {&self.char_, static_cast<SizeT>(0)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept
      -> ranges::Slice<meta::CopyCV<Self, T>, SizeT> {
    return {memory::forward<Self>(self).data(), static_cast<SizeT>(0)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  reversed_slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept
      -> ranges::ReversedSlice<meta::CopyCV<Self, T>, SizeT> {
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

  T char_ = '\0';
};

template <meta::Character T, usize N>
NTTPString(T const (&str)[N]) -> NTTPString<N - 1, T, usize>;

template <NTTPString... Strings>
constexpr NTTPString cat_nttpstrings = [] {
  NTTPString<(... + Strings.len())> ret;
  usize                             offset = 0;
  (..., (offset += memory::copy(Strings.slice(), ret.data() + offset).len()));
  return ret;
}();

} // namespace yacsl::containers
