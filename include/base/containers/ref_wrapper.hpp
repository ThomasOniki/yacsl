#pragma once

#include "base/attributes.hpp"
#include "base/memory/lifetimes.hpp"

namespace base {

template <typename T> class RefWrapper {
public:
  RefWrapper(T& value BASE_LIFETIMEBOUND) noexcept : ptr_(value) {}

  [[nodiscard]] constexpr T& get(this RefWrapper& self BASE_LIFETIMEBOUND) noexcept {
    return *self.ptr_;
  }
  [[nodiscard]] constexpr T&& get(this RefWrapper&& self BASE_LIFETIMEBOUND) noexcept {
    return base::move(*base::move(self).ptr_);
  }
  [[nodiscard]] T const& get(this RefWrapper const& self BASE_LIFETIMEBOUND) noexcept {
    return *self.ptr_;
  }
  [[nodiscard]] T const&& get(this RefWrapper const&& self BASE_LIFETIMEBOUND) noexcept {
    return base::move(*base::move(self).ptr_);
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&& operator*(this Self&& self BASE_LIFETIMEBOUND) {
    return base::forward<Self>(self).get();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto operator->(this Self&& self BASE_LIFETIMEBOUND) {
    auto&& ref = base::forward<Self>(self).get();
    return &ref;
  }

private:
  T* ptr_;
};

} // namespace base
