#pragma once

#ifndef YACSL_NO_LIBCPP
#include <initializer_list>
#endif

#include "yacsl/type_aliases.hpp"

#ifdef YACSL_NO_LIBCPP

namespace std {

// NOLINTNEXTLINE(bugprone-std-namespace-modification)
template <typename T> class initializer_list {
public:
  constexpr initializer_list() noexcept : data_(nullptr), size_(0) {}

  // There are no explicit object parameters here because some compilers might
  // rely on implicit this.
  [[nodiscard]] constexpr yacsl::usize size() const noexcept { return size_; }
  [[nodiscard]] constexpr T const*    begin() const noexcept { return data_; }
  [[nodiscard]] constexpr T const*    end() const noexcept {
    return data_ + size_;
  }

public:
  using value_type      = T;
  using reference       = T const&;
  using const_reference = T const&;
  using size_type       = yacsl::usize;

  using iterator       = T const*;
  using const_iterator = T const*;

private:
  initializer_list(T const* data, yacsl::usize size)
      : data_(data), size_(size) {}

private:
  T const*    data_;
  yacsl::usize size_;
};

} // namespace std

#endif

namespace yacsl::ranges {

template <typename T> using InitializerList = std::initializer_list<T>;

} // namespace yacsl::ranges
