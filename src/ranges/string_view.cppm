module;

#include "base/attributes.hpp"
#include <string.h>

export module base.ranges.string_view;

import base.ranges.reverse_iterator;
import base.utilities.type_aliases;
import base.utilities.is_constant_evaluated;
import base.type_traits.categories;
import base.type_traits.comparaison;

export namespace base {

template <Character T, Integral SizeT = usize> class BasicStringView {
public:
  constexpr BasicStringView() = default;
  constexpr BasicStringView(BASE_LIFETIMEBOUND const T* data, SizeT size)
      : data_(data), size_(size) {}

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr BasicStringView(BASE_LIFETIMEBOUND const T* cstr) : data_(cstr) {
    if constexpr (SameAs<char, T>) {
      if (!is_constant_evaluated()) {
        size_ = strlen(cstr);
        return;
      }
    }
    usize len = 0;
    while (cstr[len] != '\0') {
      ++len;
    }
    size_ = len;
  }

  constexpr BasicStringView(BASE_LIFETIMEBOUND const T* begin, BASE_LIFETIMEBOUND const T* end)
      : data_(begin), size_(static_cast<SizeT>(end - begin)) {}

  [[nodiscard]] constexpr const T* data() const noexcept BASE_LIFETIMEBOUND { return data_; }
  [[nodiscard]] constexpr SizeT    size() const noexcept { return size_; }

  [[nodiscard]] constexpr const T& front() const noexcept { return data_[0]; }
  [[nodiscard]] constexpr const T& back() const noexcept { return data_[size_ - 1]; }

  [[nodiscard]] constexpr const T& operator[](SizeT index) const noexcept { return data_[index]; }

  [[nodiscard]] constexpr const T* begin() const noexcept { return data_; }
  [[nodiscard]] constexpr const T* end() const noexcept { return data_ + size_; }
  [[nodiscard]] constexpr const T* cbegin() const noexcept { return data_; }
  [[nodiscard]] constexpr const T* cend() const noexcept { return data_ + size_; }

  [[nodiscard]] constexpr ReverseIterator<const T*> rbegin() const noexcept {
    return data_ + size_;
  }
  [[nodiscard]] constexpr ReverseIterator<const T*> rend() const noexcept { return data_; }
  [[nodiscard]] constexpr ReverseIterator<const T*> crbegin() const noexcept {
    return data_ + size_;
  }
  [[nodiscard]] constexpr ReverseIterator<const T*> crend() const noexcept { return data_; }

  [[nodiscard]] constexpr bool operator==(BasicStringView other) const noexcept {
    if (size_ != other.size_) {
      return false;
    }
    if (is_constant_evaluated()) {
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
  const T* data_ = nullptr;
  SizeT    size_ = 0;
};

template <Character Char, Integral SizeT>
BasicStringView(const Char*, SizeT) -> BasicStringView<Char, SizeT>;
template <Character Char> BasicStringView(const Char*) -> BasicStringView<Char, usize>;
template <Character Char> BasicStringView(const Char*, const Char*) -> BasicStringView<Char, usize>;

using StringView = BasicStringView<char>;

constexpr BasicStringView<char> operator""_sv(BASE_LIFETIMEBOUND const char* cstr, usize len) {
  return {cstr, len};
}
constexpr BasicStringView<wchar_t> operator""_sv(BASE_LIFETIMEBOUND const wchar_t* cstr,
                                                 usize                             len) {
  return {cstr, len};
}
constexpr BasicStringView<char8_t> operator""_sv(BASE_LIFETIMEBOUND const char8_t* cstr,
                                                 usize                             len) {
  return {cstr, len};
}
constexpr BasicStringView<char16_t> operator""_sv(BASE_LIFETIMEBOUND const char16_t* cstr,
                                                  usize                              len) {
  return {cstr, len};
}
constexpr BasicStringView<char32_t> operator""_sv(BASE_LIFETIMEBOUND const char32_t* cstr,
                                                  usize                              len) {
  return {cstr, len};
}

} // namespace base
