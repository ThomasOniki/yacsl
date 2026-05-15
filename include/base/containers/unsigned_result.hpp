#pragma once

#include "base/logging/common.hpp"
#include "base/logging/errors.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/sign.hpp"
#include "base/utilities/move_forward.hpp"

namespace base {

template <UnsignedIntegral Ok, Enum Err>
  requires(sizeof(Err) <= sizeof(Ok) && UnsignedIntegral<UnderlyingEnumType<Err>>)
class UnsignedResult {
public:
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr UnsignedResult(Ok expected) noexcept : value_(static_cast<Signed>(expected)) {}

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr UnsignedResult(Err error) noexcept : value_(-static_cast<Signed>(error)) {}

  [[nodiscard]] constexpr Err error() const noexcept {
    if (value_ >= 0) [[likely]] {
      return Err{0};
    } else {
      return Err(-value_);
    }
  }
  [[nodiscard]] constexpr bool is_ok() const noexcept { return value_ >= 0; }
  [[nodiscard]] constexpr bool is_error() const noexcept { return value_ < 0; }

  [[nodiscard]] constexpr Ok expect_ok(StringView error) const noexcept {
    panic_if(is_error(), error);
    return static_cast<Ok>(value_);
  }
  template <typename... Args>
  [[nodiscard]] constexpr Ok expect_ok(FormatString<Args...> fmt, Args&&... args) const noexcept {
    panic_if(is_error(), fmt, base::forward<Args>(args)...);
    return static_cast<Ok>(value_);
  }
  [[nodiscard]] constexpr Ok expect_ok() const noexcept {
    return expect_ok("UnsignedResult is not ok.");
  }

  [[nodiscard]] constexpr Ok assert_ok(StringView error) const noexcept {
    debug_panic_if(is_error(), error);
    return static_cast<Ok>(value_);
  }
  template <typename... Args>
  [[nodiscard]] constexpr Ok assert_ok(FormatString<Args...> fmt, Args&&... args) const noexcept {
    debug_panic_if(is_error(), fmt, base::forward<Args>(args)...);
    return static_cast<Ok>(value_);
  }
  [[nodiscard]] constexpr Ok assert_ok() const noexcept {
    return assert_ok("UnsignedResult is not ok.");
  }

private:
  using Signed = MakeSigned<Ok>;
  Signed value_;
};

} // namespace base
