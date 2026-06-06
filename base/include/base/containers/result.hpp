#pragma once

#include "base/containers/error.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/common.hpp"
#include "base/logging/panic.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/nil.hpp"

namespace base {

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
template <typename Ok, Enum Err> class Result {
public:
  template <typename... Args> // NOLINTNEXTLINE(google-explicit-constructor)
  Result(Args&&... args) : ok_(base::forward<Args>(args)...) {}

  // NOLINTNEXTLINE(google-explicit-constructor)
  Result(Err err) : err_(err) {}

  ~Result() noexcept { destroy(); }

  constexpr Result(const Result& other) noexcept : is_err_(other.is_err_) {
    if (other.is_err_) [[unlikely]] {
      new (&err_) Err(other.err_);
    } else {
      new (&ok_) Ok(other.ok_);
    }
  }
  constexpr Result(Result&& other) noexcept : is_err_(other.is_err_) {
    if (other.is_err_) [[unlikely]] {
      new (&err_) Ok(base::move(other.err_));
    } else {
      new (&ok_) Ok(base::move(other.ok_));
    }
  }

  constexpr Result& operator=(const Result& other) noexcept {
    if (&other == this) [[unlikely]] {
      return *this;
    }
    destroy();
    is_err_ = other.is_err_;
    if (other.is_err_) [[unlikely]] {
      new (&err_) Err(other.err_);
    } else {
      new (&ok_) Ok(other.ok_);
    }
    return *this;
  }

  constexpr Result& operator=(Result&& other) noexcept {
    destroy();
    is_err_ = other.is_err_;
    if (other.is_err_) [[unlikely]] {
      new (&err_) Ok(base::move(other.err_));
    } else {
      new (&ok_) Ok(base::move(other.ok_));
    }
    return *this;
  }

  [[nodiscard]] constexpr bool is_ok() const noexcept { return !is_err_; }
  [[nodiscard]] constexpr bool is_err() const noexcept { return is_err_; }

  [[nodiscard]] constexpr const Ok& expect_ok(StringView error) const noexcept BASE_LIFETIMEBOUND {
    panic_if(is_err_, error);
    return ok_;
  }
  template <typename... Args>
  [[nodiscard]] constexpr const Ok&
  expect_ok(const FormatString<Args...>& error_fmt,
            Args&&... error_fmt_args) const noexcept BASE_LIFETIMEBOUND {
    panic_if(is_err_, error_fmt, base::forward<Args>(error_fmt_args)...);
    return ok_;
  }
  [[nodiscard]] constexpr const Ok& expect_ok() const noexcept BASE_LIFETIMEBOUND {
    if (is_err_) [[unlikely]] {
      panic("Result<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, err_);
    }
    return ok_;
  }

  [[nodiscard]] constexpr const Ok& assert_ok(StringView error) const noexcept BASE_LIFETIMEBOUND {
    debug_panic_if(is_err_, error);
    return ok_;
  }
  template <typename... Args>
  [[nodiscard]] constexpr const Ok&
  assert_ok(const FormatString<Args...>& error_fmt,
            Args&&... error_fmt_args) const noexcept BASE_LIFETIMEBOUND {
    debug_panic_if(is_err_, error_fmt, base::forward<Args>(error_fmt_args)...);
    return ok_;
  }
  [[nodiscard]] constexpr const Ok& assert_ok() const noexcept BASE_LIFETIMEBOUND {
    if (is_err_) [[unlikely]] {
      debug_panic("Result<{}, {}> is error {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, err_);
    }
    return ok_;
  }

  [[nodiscard]] constexpr const Err&
  expect_err(StringView error) const noexcept BASE_LIFETIMEBOUND {
    panic_if(!is_err_, error);
    return err_;
  }
  template <typename... Args>
  [[nodiscard]] constexpr const Err&
  expect_err(const FormatString<Args...>& error_fmt,
             Args&&... error_fmt_args) const noexcept BASE_LIFETIMEBOUND {
    panic_if(!is_err_, error_fmt, base::forward<Args>(error_fmt_args)...);
    return err_;
  }
  [[nodiscard]] constexpr const Err& expect_err() const noexcept BASE_LIFETIMEBOUND {
    if (!is_err_) [[unlikely]] {
      panic("Result<{}, {}> is ok {}.", TYPE_STRING<Ok>, TYPE_STRING<Err>, ok_);
    }
    return err_;
  }

  [[nodiscard]] constexpr const Err&
  assert_err(StringView error) const noexcept BASE_LIFETIMEBOUND {
    debug_panic_if(!is_err_, error);
    return err_;
  }
  template <typename... Args>
  [[nodiscard]] constexpr const Err&
  assert_err(const FormatString<Args...>& error_fmt,
             Args&&... error_fmt_args) const noexcept BASE_LIFETIMEBOUND {
    debug_panic_if(!is_err_, error_fmt, base::forward<Args>(error_fmt_args)...);
    return err_;
  }
  [[nodiscard]] constexpr const Err& assert_err() const noexcept BASE_LIFETIMEBOUND {
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
// NOLINTEND(cppcoreguidelines-pro-type-union-access)

} // namespace base
