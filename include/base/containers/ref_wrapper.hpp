#pragma once

#include "base/attributes.hpp"
#include "base/memory/lifetimes.hpp"

namespace base::containers {

template <typename T> class RefWrapper {
public:
  RefWrapper(T& value BASE_LIFETIMEBOUND) noexcept : ptr_(&value) {}

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  get(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<type_traits::CopyCVRef<Self&&, T>>(
        *memory::forward<Self>(self).ptr_);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  operator*(this Self&& self BASE_LIFETIMEBOUND) {
    return memory::forward<type_traits::CopyCVRef<Self&&, T>>(
        *memory::forward<Self>(self).ptr_);
  }
  template <typename Self>
  [[nodiscard]] constexpr type_traits::CopyCV<Self, T>*
  operator->(this Self&& self BASE_LIFETIMEBOUND) {
    return memory::forward<Self>(self).ptr_;
  }

private:
  T* ptr_;
};

} // namespace base::containers
