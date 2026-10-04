#pragma once

#include <string.h>

#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/same_as.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::ranges {

template <meta::Character T, meta::Integral SizeT = usize>
class BasicStringView {
public:
  constexpr BasicStringView() = default;
  constexpr BasicStringView(T const* data YACSL_LIFETIMEBOUND, SizeT size)
      : data_(data), size_(size) {}

  constexpr BasicStringView(T const* cstr YACSL_LIFETIMEBOUND) : data_(cstr) {
    if constexpr (meta::SameAs<char, T>) {
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

  constexpr BasicStringView(T const* begin YACSL_LIFETIMEBOUND,
                            T const* end   YACSL_LIFETIMEBOUND)
      : data_(begin), size_(static_cast<SizeT>(end - begin)) {}

  [[nodiscard]] constexpr T const* data(this BasicStringView self) noexcept {
    return self.data_;
  }
  [[nodiscard]] constexpr SizeT size(this BasicStringView self) noexcept {
    return self.size_;
  }

  [[nodiscard]] constexpr T const& operator[](this BasicStringView const& self,
                                              SizeT index) noexcept {
    return self.data_[index];
  }

  [[nodiscard]] constexpr Slice<T const, SizeT>
  slice(this BasicStringView self) noexcept {
    return {self.data_, self.size_};
  }
  [[nodiscard]] constexpr ReversedSlice<T const, SizeT>
  reversed_slice(this BasicStringView self) noexcept {
    return {self.data_, self.size_};
  }
  [[nodiscard]] constexpr Slice<u8 const, SizeT>
  bytes(this BasicStringView self) noexcept {
    return {
        reinterpret_cast<u8 const*>(self.data_),
        static_cast<SizeT>(self.size_ * sizeof(T)),
    };
  }

  [[nodiscard]] constexpr T const&
  front(this BasicStringView const& self) noexcept {
    return self.data_[0];
  }
  [[nodiscard]] constexpr T const&
  back(this BasicStringView const& self) noexcept {
    return self.data_[self.size_ - 1];
  }

  [[nodiscard]] constexpr T const* begin(this BasicStringView self) noexcept {
    return self.data_;
  }
  [[nodiscard]] constexpr T const* end(this BasicStringView self) noexcept {
    return self.data_ + self.size_;
  }

  [[nodiscard]] constexpr bool operator==(this BasicStringView self,
                                          BasicStringView      other) noexcept {
    if (self.size_ != other.size_) {
      return false;
    }
    if consteval {
      for (size_t i = 0; i < self.size_; ++i) {
        if (other[i] != self.data_[i]) {
          return false;
        }
      }
      return true;
    }
    return memcmp(self.data_, other.data_, self.size_) == 0;
  }

private:
  T const* data_ = nullptr;
  SizeT    size_ = 0;
};

template <meta::Character Char, meta::Integral SizeT>
BasicStringView(Char const*, SizeT) -> BasicStringView<Char, SizeT>;
template <meta::Character Char>
BasicStringView(Char const*) -> BasicStringView<Char, usize>;
template <meta::Character Char>
BasicStringView(Char const*, Char const*) -> BasicStringView<Char, usize>;

using StringView = BasicStringView<char>;

constexpr BasicStringView<char>
operator""_sv(char const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr BasicStringView<wchar_t>
operator""_sv(wchar_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr BasicStringView<char8_t>
operator""_sv(char8_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr BasicStringView<char16_t>
operator""_sv(char16_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr BasicStringView<char32_t>
operator""_sv(char32_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}

} // namespace yacsl::ranges
