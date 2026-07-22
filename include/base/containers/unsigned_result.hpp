#pragma once

#include "base/attributes.hpp"
#include "base/formatting/format_string.hpp"
#include "base/logging/panic.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/type_traits/sign.hpp"
#include "base/type_traits/type_string.hpp"

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
      return Err(0);
    } else {
      return Err(-value_);
    }
  }
  [[nodiscard]] constexpr bool is_ok() const noexcept { return value_ >= 0; }
  [[nodiscard]] constexpr bool is_error() const noexcept { return value_ < 0; }

  template <typename... Args>
  [[nodiscard]] constexpr Ok expect_ok(FormatString<Args...> fmt,
                                       Args&&... args) const noexcept BASE_LIFETIMEBOUND {
    if (is_error()) [[unlikely]] {
      panic("UnsignedResult<{}, {}> is error {}.", Err(-value_));
      panic(fmt, base::forward<Args>(args)...);
    }
    return static_cast<Ok>(value_);
  }
  [[nodiscard]] constexpr Ok expect_ok() const noexcept BASE_LIFETIMEBOUND {
    return expect_ok("UnsignedResult<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>,
                     error());
  }

  template <typename... Args>
  [[nodiscard]] constexpr Ok assert_ok(FormatString<Args...> fmt,
                                       Args&&... args) const noexcept BASE_LIFETIMEBOUND {
    debug_panic_if(is_error(), fmt, base::forward<Args>(args)...);
    return static_cast<Ok>(value_);
  }
  [[nodiscard]] constexpr Ok assert_ok() const noexcept BASE_LIFETIMEBOUND {
    return assert_ok("UnsignedResult<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>,
                     error());
  }

private:
  using Signed = MakeSigned<Ok>;
  Signed value_;
};

} // namespace base
