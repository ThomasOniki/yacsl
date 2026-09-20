#pragma once

#include "base/attributes.hpp"
#include "base/debug/panic.hpp"
#include "base/format/format_string.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/type_traits/sign.hpp"
#include "base/type_traits/type_string.hpp"

namespace base {

template <UnsignedIntegral Ok, Enum Err>
  requires(sizeof(Err) <= sizeof(Ok) &&
           UnsignedIntegral<UnderlyingEnumType<Err>>)
class UnsignedResult {
public:
  constexpr UnsignedResult(Ok expected) noexcept
      : value_(static_cast<Signed>(expected)) {}

  constexpr UnsignedResult(Err error) noexcept
      : value_(-static_cast<Signed>(error)) {}

  [[nodiscard]] constexpr Err error(this UnsignedResult self) noexcept {
    if (self.value_ >= 0) [[likely]] {
      return Err(0);
    } else {
      return Err(-self.value_);
    }
  }
  [[nodiscard]] constexpr bool is_ok(this UnsignedResult self) noexcept {
    return self.value_ >= 0;
  }
  [[nodiscard]] constexpr bool is_error(this UnsignedResult self) noexcept {
    return self.value_ < 0;
  }

  template <typename... Args>
  [[nodiscard]] constexpr Ok expect_ok(this UnsignedResult   self,
                                       FormatString<Args...> fmt,
                                       Args&&... args) noexcept {
    if (self.is_error()) [[unlikely]] {
      panic("UnsignedResult<{}, {}> is error {}.", Err(-self.value_));
      panic(fmt, base::forward<Args>(args)...);
    }
    return static_cast<Ok>(self.value_);
  }
  [[nodiscard]] constexpr Ok expect_ok(this UnsignedResult self) noexcept {
    return self.expect_ok("UnsignedResult<{}, {}> is error {}.",
                          TYPE_STRING<Ok>, TYPE_STRING<Err>, self.error());
  }

  template <typename... Args>
  [[nodiscard]] constexpr Ok assert_ok(this UnsignedResult   self,
                                       FormatString<Args...> fmt,
                                       Args&&... args) noexcept {
    debug_panic_if(self.is_error(), fmt, base::forward<Args>(args)...);
    return static_cast<Ok>(self.value_);
  }
  [[nodiscard]] constexpr Ok assert_ok(this UnsignedResult self) noexcept {
    return self.assert_ok("UnsignedResult<{}, {}> is error {}.",
                          TYPE_STRING<Ok>, TYPE_STRING<Err>, self.error());
  }

private:
  using Signed = MakeSigned<Ok>;
  Signed value_;
};

} // namespace base
