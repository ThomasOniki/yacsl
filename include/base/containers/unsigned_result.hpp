#pragma once

#include "base/debug/assert.hpp"
#include "base/debug/panic.hpp"
#include "base/format/format_string.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/type_traits/sign.hpp"
#include "base/type_traits/type_string.hpp"

namespace base::containers {

template <type_traits::UnsignedIntegral Ok, type_traits::Enum Err>
  requires(sizeof(Err) <= sizeof(Ok) &&
           type_traits::UnsignedIntegral<type_traits::UnderlyingEnumType<Err>>)
class UnsignedResult {
public:
  constexpr UnsignedResult(Ok expected) noexcept
      : value_(static_cast<Signed>(expected)) {}

  constexpr UnsignedResult(Err error) noexcept
      : value_(-static_cast<Signed>(error)) {}

  [[nodiscard]] constexpr Err error(this UnsignedResult self) noexcept {
    if (self.value_ >= 0) [[likely]] {
      return Err(0);
    }
    return Err(-self.value_);
  }
  [[nodiscard]] constexpr bool is_ok(this UnsignedResult self) noexcept {
    return self.value_ >= 0;
  }
  [[nodiscard]] constexpr bool is_error(this UnsignedResult self) noexcept {
    return self.value_ < 0;
  }

  template <typename... Args>
  [[nodiscard]] constexpr Ok expect_ok(this UnsignedResult           self,
                                       format::FormatString<Args...> fmt,
                                       Args&&... args) noexcept {
    if (self.is_error()) [[unlikely]] {
      debug::panic("UnsignedResult<{}, {}> is error {}.", Err(-self.value_));
      debug::panic(fmt, memory::forward<Args>(args)...);
    }
    return static_cast<Ok>(self.value_);
  }
  [[nodiscard]] constexpr Ok expect_ok(this UnsignedResult self) noexcept {
    return self.expect_ok("UnsignedResult<{}, {}> is error {}.",
                          type_traits::TYPE_STRING<Ok>,
                          type_traits::TYPE_STRING<Err>, self.error());
  }

  template <typename... Args>
  [[nodiscard]] constexpr Ok assert_ok(this UnsignedResult           self,
                                       format::FormatString<Args...> fmt,
                                       Args&&... args) noexcept {
    debug::assert_true(self.is_ok(), fmt, memory::forward<Args>(args)...);
    return static_cast<Ok>(self.value_);
  }
  [[nodiscard]] constexpr Ok assert_ok(this UnsignedResult self) noexcept {
    return self.assert_ok("UnsignedResult<{}, {}> is error {}.",
                          type_traits::TYPE_STRING<Ok>,
                          type_traits::TYPE_STRING<Err>, self.error());
  }

private:
  using Signed = type_traits::MakeSigned<Ok>;
  Signed value_;
};

} // namespace base::containers
