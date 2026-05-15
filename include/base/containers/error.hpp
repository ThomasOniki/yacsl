#pragma once

#include "base/containers/fwds.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/errors.hpp"
#include "base/type_traits/categories.hpp"
#include "base/utilities/nil.hpp"

namespace base {

template <Enum Err> class Error {
public:
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Error(Nil /*nil*/) noexcept {}
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Error(Err error) noexcept : value_(error) {}

  constexpr bool operator==(Err error) const noexcept { return value_ == error; }
  constexpr bool operator==(Nil /*nil*/) const noexcept { return value_ == Err{0}; }

  constexpr Err error() const noexcept { return value_; }

  constexpr void expect_nil(StringView error_msg) const noexcept {
    if (value_ != Err{0}) [[unlikely]] {
      panic(error_msg);
    }
  }
  template <typename... Args>
  constexpr void expect_nil(const FormatString<Args...>& error_fmt, Args&&... args) {
    if (value_ != Err{0}) [[unlikely]] {
      panic(error_fmt, base::forward<Args>(args)...);
    }
  }
  constexpr void expect_nil() { expect_nil("Error is not nil."); }

  constexpr void assert_nil(StringView error_msg) const noexcept {
    if (value_ != Err{0}) [[unlikely]] {
      debug_panic(error_msg);
    }
  }
  template <typename... Args>
  constexpr void assert_nil(const FormatString<Args...>& error_fmt, Args&&... args) {
    if (value_ != Err{0}) [[unlikely]] {
      debug_panic(error_fmt, base::forward<Args>(args)...);
    }
  }
  constexpr void assert_nil() { assert_nil("Error is not nil."); }

private:
  Err value_{0};
};

template <Enum Err> Error(Err) -> Error<Err>;

} // namespace base
