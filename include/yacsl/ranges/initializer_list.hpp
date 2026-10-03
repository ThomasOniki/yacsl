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
  [[nodiscard]] constexpr yacsl::usize
  size(this initializer_list const& self) noexcept {
    return self.size_;
  }
  [[nodiscard]] constexpr T const*
  data(this initializer_list const& self) noexcept {
    return self.data_;
  }

  [[nodiscard]] constexpr T const*
  begin(this initializer_list const& self) noexcept {
    return self.data_;
  }
  [[nodiscard]] constexpr T const*
  end(this initializer_list const& self) noexcept {
    return self.data_ + self.size_;
  }

private:
  T const*     data_ = nullptr;
  yacsl::usize size_ = 0;
};

} // namespace std

#endif

namespace yacsl::ranges {

template <typename T> using InitializerList = std::initializer_list<T>;

} // namespace yacsl::ranges
