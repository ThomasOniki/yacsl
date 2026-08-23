#pragma once

#include "base/attributes.hpp"
#include "base/containers/stack_array.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename T> class StackStorage {
public:
  template <typename... Args> constexpr T& construct(this StackStorage& self, Args&&... args) {
    return *base::construct_at(self.ptr(), base::forward<Args>(args)...);
  }
  constexpr void destruct(this StackStorage& self) noexcept { self.ptr()->~T(); }

  template <typename Self>
  [[nodiscard]] constexpr auto operator->(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).ptr();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& operator*(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).get();
  }

  [[nodiscard]] constexpr T& get(this StackStorage& self BASE_LIFETIMEBOUND) noexcept {
    return *self.ptr();
  }
  [[nodiscard]] constexpr T&& get(this StackStorage&& self BASE_LIFETIMEBOUND) noexcept {
    return base::move(*base::move(self).ptr());
  }
  [[nodiscard]] constexpr T const& get(this StackStorage const& self BASE_LIFETIMEBOUND) noexcept {
    return *self.ptr();
  }
  [[nodiscard]] constexpr T const&&
  get(this StackStorage const&& self BASE_LIFETIMEBOUND) noexcept {
    return base::move(*base::move(self).ptr());
  }

private:
  [[nodiscard]] constexpr T* ptr(this StackStorage& self BASE_LIFETIMEBOUND) noexcept {
    return reinterpret_cast<T*>(self.data_.data());
  }
  [[nodiscard]] constexpr T* ptr(this StackStorage&& self BASE_LIFETIMEBOUND) noexcept {
    return reinterpret_cast<T*>(base::move(self).data_.data());
  }
  [[nodiscard]] constexpr T const* ptr(this StackStorage const& self BASE_LIFETIMEBOUND) noexcept {
    return reinterpret_cast<T const*>(base::move(self).data_.data());
  }

private:
  alignas(T) StackArray<uchar, sizeof(T)> data_{};
};

} // namespace base
