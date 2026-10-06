#pragma once

#include "yacsl/debug/panic.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/utils/nil.hpp"

namespace yacsl::containers {

template <meta::Enum Err> class Error {
public:
  constexpr Error(utils::Nil /*nil*/) noexcept {}

  constexpr Error(Err error) noexcept : value_(error) {}

  [[nodiscard]] constexpr bool operator==(this Error self, Err error) noexcept {
    return self.value_ == error;
  }
  [[nodiscard]] constexpr bool operator==(this Error self,
                                          utils::Nil /*nil*/) noexcept {
    return self.value_ == Err(0);
  }

  [[nodiscard]] constexpr Err error(this Error self) noexcept {
    return self.value_;
  }

  template <typename... Args>
  constexpr void expect_nil(this Error                        self,
                            fmt::FormatString<Args...> const& error_fmt,
                            Args&&... args) {
    if (self.value_ != Err(0)) [[unlikely]] {
      debug::panic(error_fmt, memory::forward<Args>(args)...);
    }
  }
  constexpr void expect_nil(this Error self) {
    self.expect_nil("Error is not nil.");
  }

  template <typename... Args>
  constexpr void assert_nil(this Error                        self,
                            fmt::FormatString<Args...> const& error_fmt,
                            Args&&... args) {
    if (self.value_ != Err(0)) [[unlikely]] {
      debug::unreachable(error_fmt, memory::forward<Args>(args)...);
    }
  }
  constexpr void assert_nil(this Error self) {
    self.assert_nil("Error is not nil.");
  }

private:
  Err value_ = Err{0};
};

template <meta::Enum Err> Error(Err) -> Error<Err>;

} // namespace yacsl::containers
