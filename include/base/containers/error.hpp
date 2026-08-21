#pragma once

#include "base/formatting/format_string.hpp"
#include "base/logging/panic.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_traits/categories.hpp"
#include "base/utilities/nil.hpp"

namespace base {

template <Enum Err> class Error {
public:
  constexpr Error(Nil /*nil*/) noexcept {}

  constexpr Error(Err error) noexcept : value_(error) {}

  [[nodiscard]] constexpr bool operator==(Err error) const noexcept { return value_ == error; }
  [[nodiscard]] constexpr bool operator==(Nil /*nil*/) const noexcept { return value_ == Err(0); }

  [[nodiscard]] constexpr Err error() const noexcept { return value_; }

  template <typename... Args>
  constexpr void expect_nil(FormatString<Args...> const& error_fmt, Args&&... args) {
    if (value_ != Err(0)) [[unlikely]] {
      panic(error_fmt, base::forward<Args>(args)...);
    }
  }
  constexpr void expect_nil() { expect_nil("Error is not nil."); }

  template <typename... Args>
  constexpr void assert_nil(FormatString<Args...> const& error_fmt, Args&&... args) {
    if (value_ != Err(0)) [[unlikely]] {
      debug_panic(error_fmt, base::forward<Args>(args)...);
    }
  }
  constexpr void assert_nil() { assert_nil("Error is not nil."); }

private:
  Err value_{0};
};

template <Enum Err> Error(Err) -> Error<Err>;

} // namespace base
