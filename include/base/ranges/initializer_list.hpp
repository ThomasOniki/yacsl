#pragma once

#ifndef BASE_NO_LIBCPP
#include <initializer_list>
#endif

#include "base/utilities/type_aliases.hpp"

#ifdef BASE_NO_LIBCPP

namespace std {

// NOLINTNEXTLINE(bugprone-std-namespace-modificatoin, cert-dcl58-cpp)
template <typename T> class initializer_list {
public:
  constexpr initializer_list() noexcept : data_(nullptr), size_(0) {}

  [[nodiscard]] constexpr base::usize size() const noexcept { return size_; }
  [[nodiscard]] constexpr const T*    begin() const noexcept { return data_; }
  [[nodiscard]] constexpr const T*    end() const noexcept { return data_ + size_; }

public:
  using value_type      = T;
  using reference       = const T&;
  using const_reference = const T&;
  using usizeype        = base::usize;

  using iterator       = const T*;
  using const_iterator = const T*;

private:
  initializer_list(const T* data, base::usize size) : data_(data), size_(size) {}

private:
  const T*    data_;
  base::usize size_;
};

} // namespace std

#endif

namespace base {

template <typename T> using InitializerList = std::initializer_list<T>;

} // namespace base
