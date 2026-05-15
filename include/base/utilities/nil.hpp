#pragma once

namespace base {

struct Nil {
  template <typename T> constexpr bool operator==(const T& /*non nil*/) const noexcept {
    return false;
  }

  template <> constexpr bool operator==(const Nil& /*nil*/) const noexcept { return true; }
  template <> constexpr bool operator==(const decltype(nullptr)& /*null*/) const noexcept {
    return true;
  }

  // NOLINTNEXTLINE(google-explicit-constructor)
  template <typename T> constexpr operator T*() const noexcept { return nullptr; }
};

constexpr Nil NIL;

} // namespace base
