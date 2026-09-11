#pragma once

#include "base/attributes.hpp"
#include "base/containers/stack_array.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

template <typename T> class StackStorage {
public:
  template <typename... Args>
  constexpr T& construct(this StackStorage& self, Args&&... args) {
    return *base::construct_at(self.ptr(), base::forward<Args>(args)...);
  }
  constexpr void destruct(this StackStorage& self) noexcept {
    self.ptr()->~T();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  operator->(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).ptr();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  operator*(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<Self>(self).get();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  get(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return base::forward<CopyCVRef<Self&&, T>>(
        *base::forward<Self>(self).ptr());
  }

private:
  template <typename Self>
  [[nodiscard]] constexpr auto*
  ptr(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return reinterpret_cast<CopyCV<Self, T>*>(
        base::forward<Self>(self).data_.data());
  }

private:
  alignas(T) StackArray<uchar, sizeof(T)> data_{};
};

} // namespace base
