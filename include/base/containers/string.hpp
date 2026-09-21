#pragma once

#include "base/attributes.hpp"
#include "base/containers/vector.hpp"
#include "base/memory/arena.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/slice.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_aliases.hpp"
#include "base/type_traits/allocator.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"

namespace base::containers {

template <type_traits::Character Char, usize N = 0,
          type_traits::AllocatorType Allocator = memory::MainArenaAllocator,
          type_traits::Integral      SizeT     = usize>
class BasicString {
public:
  constexpr BasicString() noexcept = default;
  constexpr explicit BasicString(Allocator const& allocator)
      : data_(allocator) {}

  constexpr ~BasicString() noexcept = default;

  constexpr BasicString(Char const*      cstr,
                        Allocator const& allocator = Allocator()) noexcept
      : data_(ranges::BasicStringView<Char, usize>(cstr), allocator) {}

  constexpr BasicString(Char const* data, SizeT len,
                        Allocator const& allocator = Allocator()) noexcept
      : data_(ranges::BasicStringView<Char, usize>(data, len), allocator) {}

  constexpr BasicString(Char const* begin, Char const* end,
                        Allocator const& allocator = Allocator()) noexcept
      : data_(ranges::BasicStringView<Char, usize>(begin, end), allocator) {}

  constexpr BasicString(ranges::BasicStringView<Char, SizeT> str,
                        Allocator const& allocator = Allocator()) noexcept
      : data_(str, allocator) {}

  constexpr BasicString(BasicString const& other) noexcept
      : data_(other.data_) {}

  constexpr BasicString(BasicString&& other) noexcept
      : data_(memory::move(other.data_)) {}

  constexpr BasicString& operator=(this BasicString& self BASE_LIFETIMEBOUND,
                                   BasicString const&     other) noexcept {
    self.data_ = other.data_;
    return self;
  }

  constexpr BasicString& operator=(this BasicString& self BASE_LIFETIMEBOUND,
                                   BasicString&&          other) noexcept {
    self.data_ = memory::move(other.data_);
    return self;
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  constexpr BasicString& append(this BasicString& self BASE_LIFETIMEBOUND,
                                Char const* ptr, SizeT len) noexcept {
    memory::copy({ptr, len}, self.grow(len));
    return self;
  }
  constexpr BasicString& append(this BasicString& self BASE_LIFETIMEBOUND,
                                Char const* begin, Char const* end) noexcept {
    return self.append(begin, static_cast<SizeT>(end - begin));
  }
  // NOLINTNEXTLINE(misc-no-recursion)
  constexpr BasicString&
  append(this BasicString& self               BASE_LIFETIMEBOUND,
         ranges::BasicStringView<Char, SizeT> str) noexcept {
    return self.append(str.data(), str.size());
  }
  constexpr BasicString& append(this BasicString& self BASE_LIFETIMEBOUND,
                                Char const*            cstr) noexcept {
    return self.append(ranges::BasicStringView<Char, usize>(cstr));
  }
  constexpr BasicString& append(this BasicString& self BASE_LIFETIMEBOUND,
                                BasicString const&     other) noexcept {
    return self.append(other.begin(), other.size());
  }

  constexpr BasicString& append(this BasicString& self BASE_LIFETIMEBOUND,
                                Char                   chr) noexcept {
    *self.grow(1) = chr;
    return self;
  }

  // NOLINTNEXTLINE(misc-no-recursion)
  constexpr BasicString& operator+=(this BasicString& self BASE_LIFETIMEBOUND,
                                    ranges::BasicStringView<Char, SizeT> str) {
    return self.append(str);
  }
  constexpr BasicString& operator+=(this BasicString& self BASE_LIFETIMEBOUND,
                                    BasicString const&     str) {
    return self.append(str);
  }
  constexpr BasicString& operator+=(this BasicString& self BASE_LIFETIMEBOUND,
                                    Char const*            cstr) {
    return self.append(cstr);
  }
  constexpr BasicString& operator+=(this BasicString& self BASE_LIFETIMEBOUND,
                                    Char                   chr) {
    return self.append(chr);
  }

  template <typename Self>
  constexpr BasicString operator+(this Self&& self, BasicString const& rhs) {
    BasicString lhs = memory::forward<Self>(self);
    return lhs += rhs;
  }
  template <typename Self>
  constexpr BasicString operator+(this Self&&                          self,
                                  ranges::BasicStringView<Char, SizeT> rhs) {
    BasicString lhs = memory::forward<Self>(self);
    return lhs += rhs;
  }
  template <typename Self>
  constexpr BasicString operator+(this Self&& self, Char const* cstr) {
    BasicString lhs = memory::forward<Self>(self);
    return lhs += cstr;
  }
  template <typename Self>
  constexpr BasicString operator+(this Self&& self, Char chr) {
    BasicString lhs = memory::forward<Self>(self);
    return lhs += chr;
  }

  constexpr BasicString& reserve(this BasicString& self BASE_LIFETIMEBOUND,
                                 SizeT                  capacity) noexcept {
    self.data_.reserve(capacity);
    return self;
  }
  constexpr BasicString& resize(this BasicString& self BASE_LIFETIMEBOUND,
                                SizeT                  size) noexcept {
    self.data_.resize(size);
    return self;
  }

  // This doesn't initialize the new memory but adds count to size. It is useful
  // for characters you want to memcpy into. It returns new_end - count.
  constexpr Char* grow(this BasicString& self BASE_LIFETIMEBOUND,
                       SizeT                  count) noexcept {
    self.data_.resize(self.data_.size() + count);
    return self.data_.end() - count;
  }

  constexpr BasicString&
  clear(this BasicString& self BASE_LIFETIMEBOUND) noexcept {
    self.data_.clear();
    return self;
  }

  template <typename Self>
  [[nodiscard]] constexpr Char*
  c_str(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    self.data_.reserve(self.data_.size() + 1);
    self.data_[self.data_.size()] = '\0';
    return memory::forward<Self>(self).data_.data();
  }

  [[nodiscard]] constexpr SizeT size(this BasicString const& self) noexcept {
    return self.data_.size();
  }
  [[nodiscard]] constexpr SizeT
  capacity(this BasicString const& self) noexcept {
    return self.data_.capacity();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&& operator[](this Self&& self BASE_LIFETIMEBOUND,
                                            SizeT            index) noexcept {
    return memory::forward<Self>(self).data_[index];
  }

  [[nodiscard]] constexpr bool operator==(this BasicString const& self,
                                          BasicString const& other) noexcept {
    return self.view() == other.view();
  }
  [[nodiscard]] constexpr bool
  operator==(this BasicString const&                     self,
             ranges::BasicStringView<Char, SizeT> const& rhs) noexcept {
    return self.view() == rhs;
  }
  [[nodiscard]] constexpr bool operator==(this BasicString const& self,
                                          Char const* cstr) noexcept {
    return self.view() == ranges::BasicStringView(cstr);
  }

  [[nodiscard]] constexpr ranges::BasicStringView<Char, SizeT>
  view(this BasicString const& self BASE_LIFETIMEBOUND) noexcept {
    return {self.data_.data(), self.data_.size()};
  }
  template <typename Self>
  [[nodiscard]] constexpr ranges::Slice<type_traits::CopyCV<Self, Char>, SizeT>
  slice(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.slice();
  }
  template <typename Self>
  [[nodiscard]] constexpr ranges::ReversedSlice<type_traits::CopyCV<Self, Char>,
                                                SizeT>
  reversed_slice(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.reversed_slice();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  bytes(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.bytes();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  data(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.data();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  begin(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.begin();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  end(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data_.end();
  }

private:
  Vector<Char, N, Allocator, SizeT> data_;
};

using String = BasicString<char>;

constexpr String operator""_s(char const* cstr BASE_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr BasicString<wchar_t>
operator""_s(wchar_t const* cstr BASE_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr BasicString<char8_t>
operator""_s(char8_t const* cstr BASE_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr BasicString<char16_t>
operator""_s(char16_t const* cstr BASE_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}
constexpr BasicString<char32_t>
operator""_s(char32_t const* cstr BASE_LIFETIMEBOUND, usize len) {
  return {cstr, len};
}

} // namespace base::containers
