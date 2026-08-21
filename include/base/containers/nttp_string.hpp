#pragma once

#include "base/attributes.hpp"
#include "base/containers/stack_array.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/reverse_iterator.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/categories.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <usize N, Character C = char, Integral SizeT = usize> class NTTPString {
public:
  constexpr NTTPString(C const (&str)[N + 1]) noexcept {
    for (usize i = 0; i < N; ++i) {
      data_[i] = str[i];
    }
  }

  constexpr NTTPString(StringView view) noexcept {
    for (usize i = 0; i < view.size(); ++i) {
      data_[i] = view[i];
    }
  }

  constexpr NTTPString() noexcept = default;

  [[nodiscard]] constexpr SizeT    size() const noexcept { return static_cast<SizeT>(N); }
  [[nodiscard]] constexpr C*       data() noexcept BASE_LIFETIMEBOUND { return data_.data(); }
  [[nodiscard]] constexpr C const* data() const noexcept BASE_LIFETIMEBOUND { return data_.data(); }

  [[nodiscard]] constexpr C& operator[](SizeT index) noexcept BASE_LIFETIMEBOUND {
    return data_[index];
  }
  [[nodiscard]] constexpr C const& operator[](SizeT index) const noexcept BASE_LIFETIMEBOUND {
    return data_[index];
  }

  [[nodiscard]] constexpr StringView view() const noexcept BASE_LIFETIMEBOUND {
    return {data_.data(), N};
  }
  [[nodiscard]] constexpr Slice<C, SizeT> slice() noexcept BASE_LIFETIMEBOUND {
    return {data_.data(), static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr Slice<C const, SizeT> slice() const noexcept BASE_LIFETIMEBOUND {
    return {data_.data(), static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept BASE_LIFETIMEBOUND {
    return {reinterpret_cast<uchar*>(data_), static_cast<SizeT>(N * sizeof(char))};
  }
  [[nodiscard]] constexpr Slice<uchar const, SizeT> bytes() const noexcept BASE_LIFETIMEBOUND {
    return {reinterpret_cast<uchar const*>(data_), static_cast<SizeT>(N * sizeof(uchar))};
  }

  [[nodiscard]] constexpr C*       begin() noexcept BASE_LIFETIMEBOUND { return data_.begin(); }
  [[nodiscard]] constexpr C*       end() noexcept BASE_LIFETIMEBOUND { return data_.end(); }
  [[nodiscard]] constexpr C const* begin() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }
  [[nodiscard]] constexpr C const* end() const noexcept BASE_LIFETIMEBOUND { return data_.end(); }
  [[nodiscard]] constexpr C const* cbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }
  [[nodiscard]] constexpr C const* cend() const noexcept BASE_LIFETIMEBOUND { return data_.end(); }

  [[nodiscard]] constexpr ReverseIterator<C*> rbegin() noexcept BASE_LIFETIMEBOUND {
    return data_.end();
  }
  [[nodiscard]] constexpr ReverseIterator<C*> rend() noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }
  [[nodiscard]] constexpr ReverseIterator<C const*> rbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_.end();
  }
  [[nodiscard]] constexpr ReverseIterator<C const*> rend() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }

  [[nodiscard]] constexpr ReverseIterator<C const*> crbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_.end();
  }
  [[nodiscard]] constexpr ReverseIterator<C const*> crend() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }

public:
  StackArray<C, N, SizeT> data_ = {'\0'};
};

template <Character C, Integral SizeT> class NTTPString<0, C, SizeT> {
public:
  constexpr NTTPString(C const (& /*str*/)[1]) noexcept {}
  constexpr NTTPString() noexcept = default;

  [[nodiscard]] constexpr SizeT    size() const noexcept { return 0; }
  [[nodiscard]] constexpr C*       data() noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr C const* data() const noexcept BASE_LIFETIMEBOUND { return &char_; }

  [[nodiscard]] constexpr C& operator[](usize /*index*/) noexcept BASE_LIFETIMEBOUND {
    return char_;
  }
  [[nodiscard]] constexpr C const& operator[](usize /*index*/) const noexcept BASE_LIFETIMEBOUND {
    return char_;
  }

  [[nodiscard]] constexpr StringView view() const noexcept BASE_LIFETIMEBOUND {
    return {&char_, static_cast<SizeT>(0)};
  }
  [[nodiscard]] constexpr Slice<C, SizeT> slice() noexcept BASE_LIFETIMEBOUND {
    return {&char_, static_cast<SizeT>(0)};
  }
  [[nodiscard]] constexpr Slice<C const, SizeT> slice() const noexcept BASE_LIFETIMEBOUND {
    return {&char_, static_cast<SizeT>(0)};
  }
  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept BASE_LIFETIMEBOUND {
    return {reinterpret_cast<uchar*>(&char_), static_cast<SizeT>(0)};
  }
  [[nodiscard]] constexpr Slice<uchar const, SizeT> bytes() const noexcept BASE_LIFETIMEBOUND {
    return {reinterpret_cast<uchar const*>(&char_), static_cast<SizeT>(0)};
  }

  [[nodiscard]] constexpr C*       begin() noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr C*       end() noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr C const* begin() const noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr C const* end() const noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr C const* cbegin() const noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr C const* cend() const noexcept BASE_LIFETIMEBOUND { return &char_; }

  [[nodiscard]] constexpr ReverseIterator<C*> rbegin() noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }
  [[nodiscard]] constexpr ReverseIterator<C*> rend() noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr ReverseIterator<C const*> rbegin() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }
  [[nodiscard]] constexpr ReverseIterator<C const*> rend() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }

  [[nodiscard]] constexpr ReverseIterator<C const*> crbegin() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }
  [[nodiscard]] constexpr ReverseIterator<C const*> crend() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
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
