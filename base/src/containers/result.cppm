module;

#include "base/attributes.hpp"

export module base.containers.result;

import base.type_traits.categories;
import base.type_traits.type_string;
import base.type_traits.trivial;
import base.memory.lifetimes;
import base.ranges.string_view;
import base.logging.panic;
import base.logging.format_string;

export namespace base {

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

  constexpr Result& operator=(const Result& other) noexcept {
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
