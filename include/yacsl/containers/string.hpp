#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/memory/allocator_type.hpp"
#include "yacsl/memory/arena.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/ranges/string_view.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::containers {

template <meta::Character T, usize N = 0,
          memory::AllocatorType A     = memory::MainArenaAllocator,
          meta::Integral        SizeT = usize>
class GenericString {
public:
  constexpr GenericString() noexcept = default;
  constexpr explicit GenericString(A const& allocator) : data_(allocator) {}

  constexpr ~GenericString() noexcept = default;

  constexpr GenericString(T const* cstr, A const& allocator = A()) noexcept
      : data_(ranges::GenericStringView<T, usize>(cstr), allocator) {}

  constexpr GenericString(T const* data, SizeT len,
                          A const& allocator = A()) noexcept
      : data_(ranges::GenericStringView<T, usize>(data, len), allocator) {}

  constexpr GenericString(T const* begin, T const* end,
                          A const& allocator = A()) noexcept
      : data_(ranges::GenericStringView<T, usize>(begin, end), allocator) {}

  constexpr GenericString(ranges::GenericStringView<T, SizeT> str,
                          A const& allocator = A()) noexcept
      : data_(str, allocator) {}

  constexpr GenericString(GenericString const& other) noexcept
      : data_(other.data_) {}

  constexpr GenericString(GenericString&& other) noexcept
      : data_(memory::move(other.data_)) {}

  constexpr GenericString&
  operator=(this GenericString& self YACSL_LIFETIMEBOUND,
            GenericString const&     other) noexcept {
    self.data_ = other.data_;
    return self;
  }

  constexpr GenericString&
  operator=(this GenericString& self YACSL_LIFETIMEBOUND,
            GenericString&&          other) noexcept {
    self.data_ = memory::move(other.data_);
    return self;
  }

  constexpr GenericString& append(this GenericString& self YACSL_LIFETIMEBOUND,
                                  T const* ptr, SizeT len) noexcept {
    memory::copy({ptr, len}, self.grow(len));
    return self;
  }
  constexpr GenericString& append(this GenericString& self YACSL_LIFETIMEBOUND,
                                  T const* begin, T const* end) noexcept {
    return self.append(begin, static_cast<SizeT>(end - begin));
  }

  constexpr GenericString&
  append(this GenericString& self            YACSL_LIFETIMEBOUND,
         ranges::GenericStringView<T, SizeT> str) noexcept {
    return self.append(str.data(), str.len());
  }
  constexpr GenericString& append(this GenericString& self YACSL_LIFETIMEBOUND,
                                  T const*                 cstr) noexcept {
    return self.append(ranges::GenericStringView<T, usize>(cstr));
  }
  constexpr GenericString& append(this GenericString& self YACSL_LIFETIMEBOUND,
                                  GenericString const&     other) noexcept {
    return self.append(other.begin(), other.len());
  }

  constexpr GenericString& append(this GenericString& self YACSL_LIFETIMEBOUND,
                                  T                        chr) noexcept {
    *self.grow(1) = chr;
    return self;
  }

  constexpr GenericString&
  operator+=(this GenericString& self            YACSL_LIFETIMEBOUND,
             ranges::GenericStringView<T, SizeT> str) {
    return self.append(str);
  }
  constexpr GenericString&
  operator+=(this GenericString& self YACSL_LIFETIMEBOUND,
             GenericString const&     str) {
    return self.append(str);
  }
  constexpr GenericString&
  operator+=(this GenericString& self YACSL_LIFETIMEBOUND, T const* cstr) {
    return self.append(cstr);
  }
  constexpr GenericString&
  operator+=(this GenericString& self YACSL_LIFETIMEBOUND, T chr) {
    return self.append(chr);
  }

  template <typename Self>
  constexpr GenericString operator+(this Self&&          self,
                                    GenericString const& rhs) {
    GenericString lhs = memory::forward<Self>(self);
    return lhs += rhs;
  }
  template <typename Self>
  constexpr GenericString operator+(this Self&&                         self,
                                    ranges::GenericStringView<T, SizeT> rhs) {
    GenericString lhs = memory::forward<Self>(self);
    return lhs += rhs;
  }
  template <typename Self>
  constexpr GenericString operator+(this Self&& self, T const* cstr) {
    GenericString lhs = memory::forward<Self>(self);
    return lhs += cstr;
  }
  template <typename Self>
  constexpr GenericString operator+(this Self&& self, T chr) {
    GenericString lhs = memory::forward<Self>(self);
    return lhs += chr;
  }

  constexpr GenericString& reserve(this GenericString& self YACSL_LIFETIMEBOUND,
                                   SizeT                    capacity) noexcept {
    self.data_.reserve(capacity);
    return self;
  }
  constexpr GenericString& resize(this GenericString& self YACSL_LIFETIMEBOUND,
                                  SizeT                    size) noexcept {
    self.data_.resize(size);
    return self;
  }

  // This doesn't initialize the new memory but adds count to size. It is useful
  // for characters you want to memcpy into. It returns new_end - count.
  constexpr T* grow(this GenericString& self YACSL_LIFETIMEBOUND,
                    SizeT                    count) noexcept {
    self.data_.resize(self.data_.len() + count);
    return self.data_.end() - count;
  }

  constexpr GenericString&
  clear(this GenericString& self YACSL_LIFETIMEBOUND) noexcept {
    self.data_.clear();
    return self;
  }

  template <typename Self>
  [[nodiscard]] constexpr T*
  c_str(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    self.data_.reserve(self.data_.len() + 1);
    self.data_[self.data_.len()] = '\0';
    return memory::forward<Self>(self).data_.data();
  }

  [[nodiscard]] constexpr SizeT len(this GenericString const& self) noexcept {
    return self.data_.len();
  }
  [[nodiscard]] constexpr SizeT
  capacity(this GenericString const& self) noexcept {
    return self.data_.capacity();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  operator[](this Self&& self YACSL_LIFETIMEBOUND, SizeT index) noexcept {
    return memory::forward<Self>(self).data_[index];
  }

  [[nodiscard]] constexpr bool operator==(this GenericString const& self,
                                          GenericString const& other) noexcept {
    return self.view() == other.view();
  }
  [[nodiscard]] constexpr bool
  operator==(this GenericString const&                  self,
             ranges::GenericStringView<T, SizeT> const& rhs) noexcept {
    return self.view() == rhs;
  }
  [[nodiscard]] constexpr bool operator==(this GenericString const& self,
                                          T const* cstr) noexcept {
    return self.view() == ranges::GenericStringView(cstr);
  }

  [[nodiscard]] constexpr ranges::GenericStringView<T, SizeT>
  view(this GenericString const& self YACSL_LIFETIMEBOUND) noexcept {
    return {self.data_.data(), self.data_.len()};
  }
  template <typename Self>
  [[nodiscard]] constexpr ranges::Slice<meta::CopyCV<Self, T>, SizeT>
  slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.slice();
  }
  template <typename Self>
  [[nodiscard]] constexpr ranges::ReversedSlice<meta::CopyCV<Self, T>, SizeT>
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
  data(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.data();
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

private:
  Vector<T, N, A, SizeT> data_;
};

using String = GenericString<char>;

constexpr String operator""_s(char const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr GenericString<wchar_t>
operator""_s(wchar_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr GenericString<char8_t>
operator""_s(char8_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr GenericString<char16_t>
operator""_s(char16_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr GenericString<char32_t>
operator""_s(char32_t const* cstr YACSL_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}

} // namespace yacsl::containers
