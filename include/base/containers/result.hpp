#pragma once

#include "base/attributes.hpp"
#include "base/formatting/format_string.hpp"
#include "base/logging/panic.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/type_traits/type_string.hpp"

namespace base {

template <typename Ok, Enum Err> class Result {
public:
  template <typename... Args> Result(Args&&... args) : ok_(base::forward<Args>(args)...) {}

  Result(Err&& err) : err_(base::move(err)), is_err_(true) {}

  Result(Err const& err) : err_(err), is_err_(true) {}

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

  constexpr Result& operator=(Result const& other) noexcept {
    if (&other == this) [[unlikely]] {
      return *this;
    }
    destroy();
    is_err_ = other.is_err_;
    if (other.is_err_) [[unlikely]] {
      base::construct_at(&err_, other.err_);
    } else {
      base::construct_at(&ok_, other.ok_);
    }
    return *this;
  }

  constexpr Result& operator=(Result&& other) noexcept {
    destroy();
    is_err_ = other.is_err_;
    if (other.is_err_) [[unlikely]] {
      base::construct_at(&err_, base::move(other.err_));
    } else {
      base::construct_at(&ok_, base::move(other.ok_));
    }
    return *this;
  }

  [[nodiscard]] constexpr bool is_ok() const noexcept { return !is_err_; }
  [[nodiscard]] constexpr bool is_err() const noexcept { return is_err_; }

  template <typename... Args>
  [[nodiscard]] constexpr Ok const&
  expect_ok(FormatString<Args...> const& error_fmt,
            Args&&... error_fmt_args) const noexcept BASE_LIFETIMEBOUND {
    if (is_err_) [[unlikely]] {
      panic("Result<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, err_);
      panic(error_fmt, base::forward<Args>(error_fmt_args)...);
    }
    return ok_;
  }
  [[nodiscard]] constexpr Ok const& expect_ok() const noexcept BASE_LIFETIMEBOUND {
    if (is_err_) [[unlikely]] {
      panic("Result<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, err_);
    }
    return ok_;
  }

  template <typename... Args>
  [[nodiscard]] constexpr Ok const&
  assert_ok(FormatString<Args...> const& error_fmt,
            Args&&... error_fmt_args) const noexcept BASE_LIFETIMEBOUND {
    if (is_err_) [[unlikely]] {
      debug_panic("Result<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, err_);
      debug_panic(error_fmt, base::forward<Args>(error_fmt_args)...);
    }
    return ok_;
  }
  [[nodiscard]] constexpr Ok const& assert_ok() const noexcept BASE_LIFETIMEBOUND {
    if (is_err_) [[unlikely]] {
      debug_panic("Result<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, err_);
    }
    return ok_;
  }

  template <typename... Args>
  [[nodiscard]] constexpr Err const&
  expect_err(FormatString<Args...> const& error_fmt,
             Args&&... error_fmt_args) const noexcept BASE_LIFETIMEBOUND {
    if (!is_err_) [[unlikely]] {
      panic("Result<{}, {}> is ok {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, ok_);
      panic(error_fmt, base::forward<Args>(error_fmt_args)...);
    }
    return err_;
  }
  [[nodiscard]] constexpr Err const& expect_err() const noexcept BASE_LIFETIMEBOUND {
    if (!is_err_) [[unlikely]] {
      panic("Result<{}, {}> is ok {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, ok_);
    }
    return err_;
  }

  template <typename... Args>
  [[nodiscard]] constexpr Err const&
  assert_err(FormatString<Args...> const& error_fmt,
             Args&&... error_fmt_args) const noexcept BASE_LIFETIMEBOUND {
    if (!is_err_) [[unlikely]] {
      debug_panic("Result<{}, {}> is ok {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, ok_);
      debug_panic(error_fmt, base::forward<Args>(error_fmt_args)...);
    }
    return err_;
  }
  [[nodiscard]] constexpr Err const& assert_err() const noexcept BASE_LIFETIMEBOUND {
    if (!is_err_) [[unlikely]] {
      debug_panic("Result<{}, {}> is ok {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, ok_);
    }
    return err_;
  }

private:
  void destroy() noexcept {
    if (is_err_) [[unlikely]] {
      err_.~Err();
    } else if constexpr (!TriviallyDestructible<Ok>) {
      ok_.~Ok();
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
