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

template <type_traits::UnsignedIntegral Value, type_traits::Enum Error>
  requires(
      sizeof(Error) <= sizeof(Value) &&
      type_traits::UnsignedIntegral<type_traits::UnderlyingEnumType<Error>>)
class UnsignedResult {
public:
  constexpr UnsignedResult(Value expected) noexcept
      : value_(static_cast<Signed>(expected)) {}

  constexpr UnsignedResult(Error error) noexcept
      : value_(-static_cast<Signed>(error)) {}

  [[nodiscard]] constexpr Error error(this UnsignedResult self) noexcept {
    if (self.value_ >= 0) [[likely]] {
      return Error(0);
    }
    return Error(-self.value_);
  }
  [[nodiscard]] constexpr bool is_value(this UnsignedResult self) noexcept {
    return self.value_ >= 0;
  }
  [[nodiscard]] constexpr bool is_error(this UnsignedResult self) noexcept {
    return self.value_ < 0;
  }

  template <typename... Args>
  [[nodiscard]] constexpr Value value(this UnsignedResult           self,
                                      format::FormatString<Args...> fmt,
                                      Args&&... args) noexcept {
    if (self.is_error()) [[unlikely]] {
      debug::panic("UnsignedResult<{}, {}> is error {}.", Error(-self.value_));
      debug::panic(fmt, memory::forward<Args>(args)...);
    }
    return static_cast<Value>(self.value_);
  }
  [[nodiscard]] constexpr Value value(this UnsignedResult self) noexcept {
    return self.value("UnsignedResult<{}, {}> is error {}.",
                      type_traits::TYPE_STRING<Value>,
                      type_traits::TYPE_STRING<Error>, self.error());
  }

  template <typename... Args>
  [[nodiscard]] constexpr Value
  value_unchecked(this UnsignedResult self, format::FormatString<Args...> fmt,
                  Args&&... args) noexcept {
    debug::assert_true(self.is_value(), fmt, memory::forward<Args>(args)...);
    return static_cast<Value>(self.value_);
  }
  [[nodiscard]] constexpr Value
  value_unchecked(this UnsignedResult self) noexcept {
    return self.value_unchecked("UnsignedResult<{}, {}> is error {}.",
                                type_traits::TYPE_STRING<Value>,
                                type_traits::TYPE_STRING<Error>, self.error());
  }

private:
  using Signed = type_traits::MakeSigned<Value>;
  Signed value_;
};

} // namespace base::containers
