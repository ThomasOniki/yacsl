#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/meta/type_string.hpp"

namespace yacsl::containers {

template <typename Value, typename Error> class Result {
public:
  template <meta::MaybeCVRef<Error> E>
  Result(E&& error) : error_(memory::forward<E>(error)), is_error_(true) {}
  template <typename... Args>
  Result(Args&&... args) : value_(memory::forward<Args>(args)...) {}

  ~Result() noexcept { destroy(); }

  constexpr Result(Result const& other) noexcept : is_error_(other.is_error_) {
    if (other.is_error_) [[unlikely]] {
      memory::construct_at(&error_, other.error_);
    } else {
      memory::construct_at(&value_, other.value_);
    }
  }
  constexpr Result(Result&& other) noexcept : is_error_(other.is_error_) {
    if (other.is_error_) [[unlikely]] {
      memory::construct_at(&error_, memory::move(other.error_));
    } else {
      memory::construct_at(&value_, memory::move(other.value_));
    }
  }

  constexpr Result& operator=(this Result& self YACSL_LIFETIMEBOUND,
                              Result const&     other) noexcept {
    if (&other == &self) [[unlikely]] {
      return self;
    }
    self.destroy();
    self.is_error_ = other.is_error_;
    if (other.is_error_) [[unlikely]] {
      memory::construct_at(&self.error_, other.error_);
    } else {
      memory::construct_at(&self.value_, other.value_);
    }
    return self;
  }

  constexpr Result& operator=(this Result& self YACSL_LIFETIMEBOUND,
                              Result&&          other) noexcept {
    self.destroy();
    self.is_error_ = other.is_error_;
    if (other.is_error_) [[unlikely]] {
      memory::construct_at(&self.error_, memory::move(other.error_));
    } else {
      memory::construct_at(&self.value_, memory::move(other.value_));
    }
    return self;
  }

  [[nodiscard]] constexpr bool is_value(this Result const& self) noexcept {
    return !self.is_error_;
  }
  [[nodiscard]] constexpr bool is_error(this Result const& self) noexcept {
    return self.is_error_;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  value(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    if (self.is_error_) [[unlikely]] {
      debug::panic("Result<{}, {}> is error {}.", meta::TYPE_STRING<Value>,
                   meta::TYPE_STRING<Error>, self.error_);
    }
    return memory::forward<Self>(self).value_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  value_unchecked(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    if (self.is_error_) [[unlikely]] {
      debug::unreachable("Result<{}, {}> is error {}.",
                         meta::TYPE_STRING<Value>, meta::TYPE_STRING<Error>,
                         self.error_);
    }
    return memory::forward<Self>(self).value_;
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  error(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    if (!self.is_error_) [[unlikely]] {
      debug::panic("Result<{}, {}> is value {}.", meta::TYPE_STRING<Value>,
                   meta::TYPE_STRING<Error>, self.value_);
    }
    return memory::forward<Self>(self).error_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  error_unchecked(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    if (!self.is_error_) [[unlikely]] {
      debug::unreachable("Result<{}, {}> is value {}.",
                         meta::TYPE_STRING<Value>, meta::TYPE_STRING<Error>,
                         self.value_);
    }
    return memory::forward<Self>(self).error_;
  }

  template <typename Self, typename F>
  auto transform(this Self&& self, F&& func)
      -> Result<decltype(func(*self.value_)), Error> {
    if (self.is_value_) {
      return memory::forward<F>(func)(*memory::forward<Self>(self).value_);
    }
    return self.error_;
  }

private:
  template <typename Self> void destroy(this Self&& self) noexcept {
    if (self.is_error_) [[unlikely]] {
      memory::forward<Self>(self).error_.~Error();
    } else {
      memory::forward<Self>(self).value_.~Value();
    }
  }

private:
  union {
    Value value_;
    Error error_;
  };
  bool is_error_ = false;
};

} // namespace yacsl::containers
