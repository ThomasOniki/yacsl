#pragma once

#include "base/attributes.hpp"
#include "base/containers/stack_array.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_aliases.hpp"
#include "base/type_traits/qualifiers.hpp"

namespace base::containers {

template <typename T> class StackStorage {
public:
  template <typename... Args>
  constexpr T& construct(this StackStorage& self, Args&&... args) {
    return *memory::construct_at(self.ptr(), memory::forward<Args>(args)...);
  }
  constexpr void destruct(this StackStorage& self) noexcept {
    self.ptr()->~T();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  operator->(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).ptr();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  operator*(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).get();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  get(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<type_traits::CopyCVRef<Self&&, T>>(
        *memory::forward<Self>(self).ptr());
  }

private:
  template <typename Self>
  [[nodiscard]] constexpr auto*
  ptr(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return reinterpret_cast<type_traits::CopyCV<Self, T>*>(
        memory::forward<Self>(self).data_.data());
  }

private:
  alignas(T) StackArray<uchar, sizeof(T)> data_{};
};

} // namespace base::containers
