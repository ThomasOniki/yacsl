#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/memory/lifetimes.hpp"

namespace yacsl::containers {

template <typename T> class RefWrapper {
public:
  RefWrapper(T& value YACSL_LIFETIMEBOUND) noexcept : ptr_(&value) {}

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  get(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<meta::CopyCVRef<Self&&, T>>(
        *memory::forward<Self>(self).ptr_);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  operator*(this Self&& self YACSL_LIFETIMEBOUND) {
    return memory::forward<meta::CopyCVRef<Self&&, T>>(
        *memory::forward<Self>(self).ptr_);
  }
  template <typename Self>
  [[nodiscard]] constexpr meta::CopyCV<Self, T>*
  operator->(this Self&& self YACSL_LIFETIMEBOUND) {
    return memory::forward<Self>(self).ptr_;
  }

private:
  T* ptr_;
};

} // namespace yacsl::containers
