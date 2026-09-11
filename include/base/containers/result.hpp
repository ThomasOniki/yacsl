#pragma once

#include "base/attributes.hpp"
#include "base/formatting/format_string.hpp"
#include "base/logging/panic.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/type_traits/type_string.hpp"

namespace base {

template <typename Ok, Enum Err> class Result {
public:
  template <MaybeCVRef<Err> E> Result(E&& err) : err_(base::forward<E>(err)), is_err_(true) {}
  template <typename... Args> Result(Args&&... args) : ok_(base::forward<Args>(args)...) {}

  ~Result() noexcept { destroy(); }

  constexpr Result(Result const& other) noexcept : is_err_(other.is_err_) {
    if (other.is_err_) [[unlikely]] {
      base::construct_at(&err_, other.err_);
    } else {
      base::construct_at(&ok_, other.ok_);
    }
  }
  constexpr Result(Result&& other) noexcept : is_err_(other.is_err_) {
    if (other.is_err_) [[unlikely]] {
      base::construct_at(&err_, base::move(other.err_));
    } else {
      base::construct_at(&ok_, base::move(other.ok_));
    }
  }

  constexpr Result& operator=(this Result& self BASE_LIFETIMEBOUND, Result const& other) noexcept {
    if (&other == &self) [[unlikely]] {
      return self;
    }
    self.destroy();
    self.is_err_ = other.is_err_;
    if (other.is_err_) [[unlikely]] {
      base::construct_at(&self.err_, other.err_);
    } else {
      base::construct_at(&self.ok_, other.ok_);
    }
    return self;
  }

  constexpr Result& operator=(this Result& self BASE_LIFETIMEBOUND, Result&& other) noexcept {
    self.destroy();
    self.is_err_ = other.is_err_;
    if (other.is_err_) [[unlikely]] {
      base::construct_at(&self.err_, base::move(other.err_));
    } else {
      base::construct_at(&self.ok_, base::move(other.ok_));
    }
    return self;
  }

  [[nodiscard]] constexpr bool is_ok(this Result const& self) noexcept { return !self.is_err_; }
  [[nodiscard]] constexpr bool is_err(this Result const& self) noexcept { return self.is_err_; }

  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&& expect_ok(this Self&& self             BASE_LIFETIMEBOUND,
                                           FormatString<Args...> const& error_fmt,
                                           Args&&... error_fmt_args) noexcept {
    if (self.is_err_) [[unlikely]] {
      panic("Result<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, self.err_);
      panic(error_fmt, base::forward<Args>(error_fmt_args)...);
    }
    return base::forward<Self>(self).ok_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& expect_ok(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    if (self.is_err_) [[unlikely]] {
      panic("Result<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, self.err_);
    }
    return base::forward<Self>(self).ok_;
  }

  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&& assert_ok(this Self&& self             BASE_LIFETIMEBOUND,
                                           FormatString<Args...> const& error_fmt,
                                           Args&&... error_fmt_args) noexcept {
    if (self.is_err_) [[unlikely]] {
      debug_panic("Result<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, self.err_);
      debug_panic(error_fmt, base::forward<Args>(error_fmt_args)...);
    }
    return base::forward<Self>(self).ok_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& assert_ok(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    if (self.is_err_) [[unlikely]] {
      debug_panic("Result<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, self.err_);
    }
    return base::forward<Self>(self).ok_;
  }

  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&& expect_err(this Self&& self             BASE_LIFETIMEBOUND,
                                            FormatString<Args...> const& error_fmt,
                                            Args&&... error_fmt_args) noexcept {
    if (!self.is_err_) [[unlikely]] {
      panic("Result<{}, {}> is ok {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, self.ok_);
      panic(error_fmt, base::forward<Args>(error_fmt_args)...);
    }
    return base::forward<Self>(self).err_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& expect_err(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    if (!self.is_err_) [[unlikely]] {
      panic("Result<{}, {}> is ok {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, self.ok_);
    }
    return base::forward<Self>(self).err_;
  }

  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&& assert_err(this Self&& self             BASE_LIFETIMEBOUND,
                                            FormatString<Args...> const& error_fmt,
                                            Args&&... error_fmt_args) noexcept {
    if (!self.is_err_) [[unlikely]] {
      debug_panic("Result<{}, {}> is ok {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, self.ok_);
      debug_panic(error_fmt, base::forward<Args>(error_fmt_args)...);
    }
    return base::forward<Self>(self).err_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& assert_err(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    if (!self.is_err_) [[unlikely]] {
      debug_panic("Result<{}, {}> is ok {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, self.ok_);
    }
    return base::forward<Self>(self).err_;
  }

private:
  template <typename Self> void destroy(this Self&& self) noexcept {
    if (self.is_err_) [[unlikely]] {
      base::forward<Self>(self).err_.~Err();
    } else if constexpr (!TriviallyDestructible<Ok>) {
      base::forward<Self>(self).ok_.~Ok();
    }
  }

private:
  union {
    Ok  ok_;
    Err err_;
  };
  bool is_err_ = false;
};

} // namespace base
