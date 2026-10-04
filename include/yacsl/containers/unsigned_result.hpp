#pragma once

#include "yacsl/debug/assert.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/fmt/format_string.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/meta/sign.hpp"
#include "yacsl/meta/type_string.hpp"

namespace yacsl::containers {

template <meta::UnsignedIntegral Value, meta::Enum Error>
  requires(sizeof(Error) <= sizeof(Value) &&
           meta::UnsignedIntegral<meta::UnderlyingEnumType<Error>>)
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
  [[nodiscard]] constexpr Value value(this UnsignedResult        self,
                                      fmt::FormatString<Args...> fmt,
                                      Args&&... args) noexcept {
    if (self.is_error()) [[unlikely]] {
      debug::panic("UnsignedResult<{}, {}> is error {}.", Error(-self.value_));
      debug::panic(fmt, memory::forward<Args>(args)...);
    }
    return static_cast<Value>(self.value_);
  }
  [[nodiscard]] constexpr Value value(this UnsignedResult self) noexcept {
    return self.value("UnsignedResult<{}, {}> is error {}.",
                      meta::TYPE_STRING<Value>, meta::TYPE_STRING<Error>,
                      self.error());
  }

  template <typename... Args>
  [[nodiscard]] constexpr Value value_unchecked(this UnsignedResult        self,
                                                fmt::FormatString<Args...> fmt,
                                                Args&&... args) noexcept {
    debug::assert_(self.is_value(), fmt, memory::forward<Args>(args)...);
    return static_cast<Value>(self.value_);
  }
  [[nodiscard]] constexpr Value
  value_unchecked(this UnsignedResult self) noexcept {
    return self.value_unchecked("UnsignedResult<{}, {}> is error {}.",
                                meta::TYPE_STRING<Value>,
                                meta::TYPE_STRING<Error>, self.error());
  }

private:
  using Signed = meta::MakeSigned<Value>;
  Signed value_;
};

} // namespace yacsl::containers
