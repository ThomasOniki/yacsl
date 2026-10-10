#pragma once

#include <string.h>

#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/same_as.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::ranges {

template <meta::Character T, meta::Integral SizeT = usize>
class GenericStringView {
public:
  constexpr GenericStringView() = default;
  constexpr GenericStringView(T const* data YACSL_LIFETIMEBOUND, SizeT size)
      : data_(data), len_(size) {}

  constexpr GenericStringView(T const* cstr YACSL_LIFETIMEBOUND) : data_(cstr) {
    if constexpr (meta::SameAs<char, T>) {
      if !consteval {
        len_ = static_cast<SizeT>(strlen(cstr));
        return;
      }
    }
    usize len = 0;
    while (cstr[len] != '\0') {
      ++len;
    }
    len_ = len;
  }

  constexpr GenericStringView(T const* begin YACSL_LIFETIMEBOUND,
                              T const* end   YACSL_LIFETIMEBOUND)
      : data_(begin), len_(static_cast<SizeT>(end - begin)) {}

  [[nodiscard]] constexpr T const* data(this GenericStringView self) noexcept {
    return self.data_;
  }
  [[nodiscard]] constexpr SizeT len(this GenericStringView self) noexcept {
    return self.len_;
  }

  [[nodiscard]] constexpr T const& operator[](this GenericStringView self,
                                              SizeT index) noexcept {
    return self.data_[index];
  }

  [[nodiscard]] constexpr Slice<T const, SizeT>
  slice(this GenericStringView self) noexcept {
    return {self.data_, self.len_};
  }
  [[nodiscard]] constexpr ReversedSlice<T const, SizeT>
  reversed_slice(this GenericStringView self) noexcept {
    return {self.data_, self.len_};
  }
  [[nodiscard]] constexpr Slice<u8 const, SizeT>
  bytes(this GenericStringView self) noexcept {
    return {
        reinterpret_cast<u8 const*>(self.data_),
        static_cast<SizeT>(self.len_ * sizeof(T)),
    };
  }

  [[nodiscard]] constexpr T const& first(this GenericStringView self) noexcept {
    return self.data_[0];
  }
  [[nodiscard]] constexpr T const& last(this GenericStringView self) noexcept {
    return self.data_[self.len_ - 1];
  }
  [[nodiscard]] constexpr GenericStringView
  subview(this GenericStringView self, SizeT start, SizeT len) noexcept {
    return {self.data_ + start, len};
  }
  [[nodiscard]] constexpr GenericStringView first_n(this GenericStringView self,
                                                    SizeT count) noexcept {
    return {self.data_, count};
  }
  [[nodiscard]] constexpr GenericStringView last_n(this GenericStringView self,
                                                   SizeT count) noexcept {
    return {self.data_ + self.len_ - count, count};
  }

  [[nodiscard]] constexpr T const* begin(this GenericStringView self) noexcept {
    return self.data_;
  }
  [[nodiscard]] constexpr T const* end(this GenericStringView self) noexcept {
    return self.data_ + self.len_;
  }

  [[nodiscard]] constexpr bool operator==(this GenericStringView self,
                                          GenericStringView other) noexcept {
    if (self.len_ != other.len_) {
      return false;
    }
    if consteval {
      for (size_t i = 0; i < self.len_; ++i) {
        if (other[i] != self.data_[i]) {
          return false;
        }
      }
      return true;
    }
    return memcmp(self.data_, other.data_, self.len_) == 0;
  }

  [[nodiscard]] constexpr bool starts_with(this GenericStringView self,
                                           GenericStringView prefix) noexcept {
    return self.len_ >= prefix.len_ && self.first_n(prefix.len_) == prefix;
  }
  [[nodiscard]] constexpr bool ends_with(this GenericStringView self,
                                         GenericStringView prefix) noexcept {
    return self.len_ >= prefix.len_ && self.last_n(prefix.len_) == prefix;
  }

  [[nodiscard]] constexpr GenericStringView
  without_prefix(this GenericStringView self, usize len) noexcept {
    return {self.data_ + len, self.len_ - len};
  }
  [[nodiscard]] constexpr GenericStringView
  without_suffix(this GenericStringView self, usize len) noexcept {
    return {self.data_, self.len_ - len};
  }

  constexpr void remove_prefix(this GenericStringView& self,
                               usize                   len) noexcept {
    self.data_ += len;
    self.len_ -= len;
  }
  constexpr void remove_suffix(this GenericStringView& self,
                               usize                   len) noexcept {
    self.len_ -= len;
  }

  constexpr bool remove_prefix(this GenericStringView& self,
                               GenericStringView       prefix) noexcept {
    if (self.starts_with(prefix)) {
      self.remove_prefix(prefix.len_);
      return true;
    }
    return false;
  }
  constexpr bool remove_suffix(this GenericStringView& self,
                               GenericStringView       suffix) noexcept {
    if (self.ends_with(suffix)) {
      self.remove_suffix(suffix.len_);
      return true;
    }
    return false;
  }

private:
  T const* data_ = nullptr;
  SizeT    len_  = 0;
};

template <meta::Character T, meta::Integral SizeT>
GenericStringView(T const*, SizeT) -> GenericStringView<T, SizeT>;
template <meta::Character T>
GenericStringView(T const*) -> GenericStringView<T, usize>;
template <meta::Character T>
GenericStringView(T const*, T const*) -> GenericStringView<T, usize>;

using StringView = GenericStringView<char>;

constexpr GenericStringView<char>
operator""_sv(char const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr GenericStringView<wchar_t>
operator""_sv(wchar_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr GenericStringView<char8_t>
operator""_sv(char8_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr GenericStringView<char16_t>
operator""_sv(char16_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr GenericStringView<char32_t>
operator""_sv(char32_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}

} // namespace yacsl::ranges
