#pragma once

#include "base/format/format_string.hpp"
#include "base/log/panic.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_traits/categories.hpp"
#include "base/utilities/nil.hpp"

namespace base {

template <Enum Err> class Error {
public:
  constexpr Error(Nil /*nil*/) noexcept {}

  constexpr Error(Err error) noexcept : value_(error) {}

  [[nodiscard]] constexpr bool operator==(this Error self, Err error) noexcept {
    return self.value_ == error;
  }
  [[nodiscard]] constexpr bool operator==(this Error self,
                                          Nil /*nil*/) noexcept {
    return self.value_ == Err(0);
  }

  [[nodiscard]] constexpr Err error(this Error self) noexcept {
    return self.value_;
  }

  template <typename... Args>
  constexpr void expect_nil(this Error                   self,
                            FormatString<Args...> const& error_fmt,
                            Args&&... args) {
    if (self.value_ != Err(0)) [[unlikely]] {
      panic(error_fmt, base::forward<Args>(args)...);
    }
  }
  constexpr void expect_nil(this Error self) {
    self.expect_nil("Error is not nil.");
  }

  template <typename... Args>
  constexpr void assert_nil(this Error                   self,
                            FormatString<Args...> const& error_fmt,
                            Args&&... args) {
    if (self.value_ != Err(0)) [[unlikely]] {
      debug_panic(error_fmt, base::forward<Args>(args)...);
    }
  }
  constexpr void assert_nil(this Error self) {
    self.assert_nil("Error is not nil.");
  }

private:
  Err value_{0};
};

template <Enum Err> Error(Err) -> Error<Err>;

} // namespace base
