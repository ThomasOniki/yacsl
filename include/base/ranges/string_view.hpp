#pragma once

#include <string.h>

#include "base/ranges/slice.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/comparaison.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <Character T, Integral SizeT = usize> class BasicStringView {
public:
  constexpr BasicStringView() = default;
  constexpr BasicStringView(T const* data, SizeT size) : data_(data), size_(size) {}

  constexpr BasicStringView(T const* cstr) : data_(cstr) {
    if constexpr (SameAs<char, T>) {
      if !consteval {
        size_ = static_cast<SizeT>(strlen(cstr));
        return;
      }
    }
    usize len = 0;
    while (cstr[len] != '\0') {
      ++len;
    }
    size_ = len;
  }

  constexpr BasicStringView(T const* begin, T const* end)
      : data_(begin), size_(static_cast<SizeT>(end - begin)) {}

  [[nodiscard]] constexpr Slice<T const, SizeT> slice() const noexcept { return {data_, size_}; }
  [[nodiscard]] constexpr ReversedSlice<T const, SizeT> reversed_slice() const noexcept {
    return {data_, size_};
  }
  [[nodiscard]] constexpr Slice<uchar const, SizeT> bytes() const noexcept {
    return {reinterpret_cast<uchar const*>(data_), static_cast<SizeT>(size_ * sizeof(T))};
  }

  [[nodiscard]] constexpr T const* data() const noexcept { return data_; }
  [[nodiscard]] constexpr SizeT    size() const noexcept { return size_; }

  [[nodiscard]] constexpr T const& front() const noexcept { return data_[0]; }
  [[nodiscard]] constexpr T const& back() const noexcept { return data_[size_ - 1]; }

  [[nodiscard]] constexpr T const& operator[](SizeT index) const noexcept { return data_[index]; }

  [[nodiscard]] constexpr T const* begin() const noexcept { return data_; }
  [[nodiscard]] constexpr T const* end() const noexcept { return data_ + size_; }

  [[nodiscard]] constexpr bool operator==(BasicStringView other) const noexcept {
    if (size_ != other.size_) {
      return false;
    }
    if consteval {
      for (size_t i = 0; i < size_; ++i) {
        if (other[i] != data_[i]) {
          return false;
        }
      }
      return true;
    }
    return memcmp(data_, other.data_, size_) == 0;
  }

private:
  T const* data_ = nullptr;
  SizeT    size_ = 0;
};

template <Character Char, Integral SizeT>
BasicStringView(Char const*, SizeT) -> BasicStringView<Char, SizeT>;
template <Character Char> BasicStringView(Char const*) -> BasicStringView<Char, usize>;
template <Character Char> BasicStringView(Char const*, Char const*) -> BasicStringView<Char, usize>;

using StringView = BasicStringView<char>;

inline namespace litterals {

constexpr BasicStringView<char> operator""_sv(char const* cstr, usize len) { return {cstr, len}; }
constexpr BasicStringView<wchar_t> operator""_sv(wchar_t const* cstr, usize len) {
  return {cstr, len};
}
constexpr BasicStringView<char8_t> operator""_sv(char8_t const* cstr, usize len) {
  return {cstr, len};
}
constexpr BasicStringView<char16_t> operator""_sv(char16_t const* cstr, usize len) {
  return {cstr, len};
}
constexpr BasicStringView<char32_t> operator""_sv(char32_t const* cstr, usize len) {
  return {cstr, len};
}

} // namespace litterals

} // namespace base
