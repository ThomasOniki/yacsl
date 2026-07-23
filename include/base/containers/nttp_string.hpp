#pragma once

#include "base/attributes.hpp"
#include "base/containers/stack_array.hpp"
#include "base/ranges/reverse_iterator.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/categories.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <usize N, Character C = char, Integral SizeT = usize> class NTTPString {
public:
  // NOLINTNEXTLINE(modernize-avoid-c-arrays, google-explicit-constructor)
  constexpr NTTPString(const C (&str)[N + 1]) noexcept {
    for (usize i = 0; i < N; ++i) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
      data_[i] = str[i];
    }
  }

  // NOLINTNEXTLINE(modernize-avoid-c-arrays, google-explicit-constructor)
  constexpr NTTPString(StringView view) noexcept {
    for (usize i = 0; i < view.size(); ++i) {
      data_[i] = view[i];
    }
  }

  constexpr NTTPString() noexcept = default;

  [[nodiscard]] constexpr SizeT    size() const noexcept { return static_cast<SizeT>(N); }
  [[nodiscard]] constexpr C*       data() noexcept BASE_LIFETIMEBOUND { return data_.data(); }
  [[nodiscard]] constexpr const C* data() const noexcept BASE_LIFETIMEBOUND { return data_.data(); }

  [[nodiscard]] constexpr C& operator[](SizeT index) noexcept BASE_LIFETIMEBOUND {
    return data_[index];
  }
  [[nodiscard]] constexpr const C& operator[](SizeT index) const noexcept BASE_LIFETIMEBOUND {
    return data_[index];
  }

  [[nodiscard]] constexpr StringView view() const noexcept BASE_LIFETIMEBOUND {
    return {data_.data(), N};
  }
  [[nodiscard]] constexpr Slice<C, SizeT> slice() noexcept BASE_LIFETIMEBOUND {
    return {data_.data(), static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr Slice<const C, SizeT> slice() const noexcept BASE_LIFETIMEBOUND {
    return {data_.data(), static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<uchar*>(data_), static_cast<SizeT>(N * sizeof(char))};
  }
  [[nodiscard]] constexpr Slice<const uchar, SizeT> bytes() const noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const uchar*>(data_), static_cast<SizeT>(N * sizeof(uchar))};
  }

  [[nodiscard]] constexpr C*       begin() noexcept BASE_LIFETIMEBOUND { return data_.begin(); }
  [[nodiscard]] constexpr C*       end() noexcept BASE_LIFETIMEBOUND { return data_.end(); }
  [[nodiscard]] constexpr const C* begin() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }
  [[nodiscard]] constexpr const C* end() const noexcept BASE_LIFETIMEBOUND { return data_.end(); }
  [[nodiscard]] constexpr const C* cbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }
  [[nodiscard]] constexpr const C* cend() const noexcept BASE_LIFETIMEBOUND { return data_.end(); }

  [[nodiscard]] constexpr ReverseIterator<C*> rbegin() noexcept BASE_LIFETIMEBOUND {
    return data_.end();
  }
  [[nodiscard]] constexpr ReverseIterator<C*> rend() noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }
  [[nodiscard]] constexpr ReverseIterator<const C*> rbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_.end();
  }
  [[nodiscard]] constexpr ReverseIterator<const C*> rend() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }

  [[nodiscard]] constexpr ReverseIterator<const C*> crbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_.end();
  }
  [[nodiscard]] constexpr ReverseIterator<const C*> crend() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }

public:
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  StackArray<C, N, SizeT> data_ = {'\0'};
};

template <Character C, Integral SizeT> class NTTPString<0, C, SizeT> {
public:
  // NOLINTNEXTLINE(modernize-avoid-c-arrays,google-explicit-constructor)
  constexpr NTTPString(const C (& /*str*/)[1]) noexcept {}
  constexpr NTTPString() noexcept = default;

  [[nodiscard]] static constexpr SizeT size() noexcept { return 0; }
  [[nodiscard]] constexpr C*           data() noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr const C*     data() const noexcept BASE_LIFETIMEBOUND { return &char_; }

  [[nodiscard]] constexpr C& operator[](usize /*index*/) noexcept BASE_LIFETIMEBOUND {
    return char_;
  }
  [[nodiscard]] constexpr const C& operator[](usize /*index*/) const noexcept BASE_LIFETIMEBOUND {
    return char_;
  }

  [[nodiscard]] constexpr StringView view() const noexcept BASE_LIFETIMEBOUND {
    return {&char_, static_cast<SizeT>(0)};
  }
  [[nodiscard]] constexpr Slice<C, SizeT> slice() noexcept BASE_LIFETIMEBOUND {
    return {&char_, static_cast<SizeT>(0)};
  }
  [[nodiscard]] constexpr Slice<const C, SizeT> slice() const noexcept BASE_LIFETIMEBOUND {
    return {&char_, static_cast<SizeT>(0)};
  }
  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<uchar*>(&char_), static_cast<SizeT>(0)};
  }
  [[nodiscard]] constexpr Slice<const uchar, SizeT> bytes() const noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const uchar*>(&char_), static_cast<SizeT>(0)};
  }

  [[nodiscard]] constexpr C*       begin() noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr C*       end() noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr const C* begin() const noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr const C* end() const noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr const C* cbegin() const noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr const C* cend() const noexcept BASE_LIFETIMEBOUND { return &char_; }

  [[nodiscard]] constexpr ReverseIterator<C*> rbegin() noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }
  [[nodiscard]] constexpr ReverseIterator<C*> rend() noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr ReverseIterator<const C*> rbegin() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }
  [[nodiscard]] constexpr ReverseIterator<const C*> rend() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }

  [[nodiscard]] constexpr ReverseIterator<const C*> crbegin() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }
  [[nodiscard]] constexpr ReverseIterator<const C*> crend() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }

  C char_ = '\0';
};

template <Character C, usize N> // NOLINTNEXTLINE(modernize-avoid-c-arrays)
NTTPString(const C (&str)[N]) -> NTTPString<N - 1, C, usize>;

} // namespace base
