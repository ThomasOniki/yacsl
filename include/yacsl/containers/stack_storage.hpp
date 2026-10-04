#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::containers {

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
  operator->(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).ptr();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  operator*(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).get();
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  get(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<meta::CopyCVRef<Self&&, T>>(
        *memory::forward<Self>(self).ptr());
  }

private:
  template <typename Self>
  [[nodiscard]] constexpr auto*
  ptr(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return reinterpret_cast<meta::CopyCV<Self, T>*>(
        memory::forward<Self>(self).data_.data());
  }

private:
  alignas(T) StackArray<u8, sizeof(T)> data_{};
};

} // namespace yacsl::containers
