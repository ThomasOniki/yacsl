#pragma once

#include "base/attributes.hpp"
#include "base/containers/stack_array.hpp"
#include "base/ranges/reverse_iterator.hpp"
#include "base/ranges/string_view.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <usize N> class NTTPString {
public:
  // NOLINTNEXTLINE(modernize-avoid-c-arrays, google-explicit-constructor)
  constexpr NTTPString(const char (&str)[N + 1]) noexcept {
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

  [[nodiscard]] constexpr usize       size() const noexcept { return N; }
  [[nodiscard]] constexpr char*       data() noexcept BASE_LIFETIMEBOUND { return data_.data(); }
  [[nodiscard]] constexpr const char* data() const noexcept BASE_LIFETIMEBOUND {
    return data_.data();
  }

  [[nodiscard]] constexpr char& operator[](usize index) noexcept BASE_LIFETIMEBOUND {
    return data_[index];
  }
  [[nodiscard]] constexpr const char& operator[](usize index) const noexcept BASE_LIFETIMEBOUND {
    return data_[index];
  }

  [[nodiscard]] constexpr StringView view() const noexcept BASE_LIFETIMEBOUND {
    return {data_.data(), N};
  }

  [[nodiscard]] constexpr char*       begin() noexcept BASE_LIFETIMEBOUND { return data_.begin(); }
  [[nodiscard]] constexpr char*       end() noexcept BASE_LIFETIMEBOUND { return data_.end(); }
  [[nodiscard]] constexpr const char* begin() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }
  [[nodiscard]] constexpr const char* end() const noexcept BASE_LIFETIMEBOUND {
    return data_.end();
  }
  [[nodiscard]] constexpr const char* cbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }
  [[nodiscard]] constexpr const char* cend() const noexcept BASE_LIFETIMEBOUND {
    return data_.end();
  }

  [[nodiscard]] constexpr ReverseIterator<char*> rbegin() noexcept BASE_LIFETIMEBOUND {
    return data_.end();
  }
  [[nodiscard]] constexpr ReverseIterator<char*> rend() noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }
  [[nodiscard]] constexpr ReverseIterator<const char*> rbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_.end();
  }
  [[nodiscard]] constexpr ReverseIterator<const char*> rend() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }

  [[nodiscard]] constexpr ReverseIterator<const char*> crbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_.end();
  }
  [[nodiscard]] constexpr ReverseIterator<const char*> crend() const noexcept BASE_LIFETIMEBOUND {
    return data_.begin();
  }

public:
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  StackArray<char, N, usize> data_ = {'\0'};
};

template <> class NTTPString<0> {
public:
  // NOLINTNEXTLINE(modernize-avoid-c-arrays,google-explicit-constructor)
  constexpr NTTPString(const char (& /*str*/)[1]) noexcept {}
  constexpr NTTPString() noexcept = default;

  [[nodiscard]] static constexpr usize size() noexcept { return 0; }
  [[nodiscard]] constexpr char*        data() noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr const char*  data() const noexcept BASE_LIFETIMEBOUND { return &char_; }

  [[nodiscard]] constexpr char& operator[](usize /*index*/) noexcept BASE_LIFETIMEBOUND {
    return char_;
  }
  [[nodiscard]] constexpr const char&
  operator[](usize /*index*/) const noexcept BASE_LIFETIMEBOUND {
    return char_;
  }

  [[nodiscard]] constexpr StringView view() const noexcept BASE_LIFETIMEBOUND {
    return {&char_, static_cast<usize>(0)};
  }

  [[nodiscard]] constexpr char*       begin() noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr char*       end() noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr const char* begin() const noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr const char* end() const noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr const char* cbegin() const noexcept BASE_LIFETIMEBOUND { return &char_; }
  [[nodiscard]] constexpr const char* cend() const noexcept BASE_LIFETIMEBOUND { return &char_; }

  [[nodiscard]] constexpr ReverseIterator<char*> rbegin() noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }
  [[nodiscard]] constexpr ReverseIterator<char*> rend() noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }
  [[nodiscard]] constexpr ReverseIterator<const char*> rbegin() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }
  [[nodiscard]] constexpr ReverseIterator<const char*> rend() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }

  [[nodiscard]] constexpr ReverseIterator<const char*> crbegin() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }
  [[nodiscard]] constexpr ReverseIterator<const char*> crend() const noexcept BASE_LIFETIMEBOUND {
    return &char_;
  }

  char char_ = '\0';
};

template <usize N> // NOLINTNEXTLINE(modernize-avoid-c-arrays)
NTTPString(const char (&str)[N]) -> NTTPString<N - 1>;

} // namespace base
