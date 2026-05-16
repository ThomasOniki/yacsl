#pragma once

namespace base {

struct Nil {
  template <typename T> constexpr bool operator==(const T& /*non nil*/) const noexcept {
    return false;
  }

  constexpr bool operator==(Nil /*nil*/) const noexcept { return true; }
  constexpr bool operator==(decltype(nullptr) /*null*/) const noexcept { return true; }

  // NOLINTNEXTLINE(google-explicit-constructor)
  template <typename T> constexpr operator T*() const noexcept { return nullptr; }
};

constexpr Nil NIL;

} // namespace base
