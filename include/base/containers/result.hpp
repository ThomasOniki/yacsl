#pragma once

#include "base/containers/error.hpp"
#include "base/containers/fwds.hpp"
#include "base/containers/string_view.hpp"
#include "base/logging/errors.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
template <typename Ok, Enum Err> class Result {
public:
  template <typename... Args> // NOLINTNEXTLINE(google-explicit-constructor)
  Result(Args&&... args) : ok_(base::forward<Args>(args)...) {}

  // NOLINTNEXTLINE(google-explicit-constructor)
  Result(Err error) : status_(error) {}

  ~Result() noexcept {
    if constexpr (!TriviallyDestructible<Ok>) {
      if (is_ok()) [[likely]] {
        ok_.~Ok();
      }
    }
  }

  constexpr Result(const Result& other) noexcept : status_(other.status_) {
    if (is_ok()) {
      new (&ok_) Ok(other.ok_);
    }
  }
  constexpr Result(Result&& other) noexcept : status_(other.status_) {
    if (is_ok()) {
      new (&ok_) Ok(base::move(other.ok_));
    }
  }
  constexpr Result& operator=(const Result& other) noexcept {
    if (&other == this) [[unlikely]] {
      return *this;
    }
    if (is_ok()) {
      ok_.~Ok();
    }
    is_ok() = other.is_ok();
    if (is_ok()) {
      ok_ = other.ok_;
    }
    return *this;
  }
  constexpr Result& operator=(Result&& other) noexcept {
    if (is_ok()) {
      ok_.~Ok();
    }
    is_ok() = other.is_ok();
    if (is_ok()) {
      ok_ = base::move(other.ok_);
    }
    return *this;
  }

  [[nodiscard]] constexpr Err  error() const noexcept { return status_; }
  [[nodiscard]] constexpr bool is_ok() const noexcept { return status_ == NIL; }
  [[nodiscard]] constexpr bool is_error() const noexcept { return status_ != NIL; }

  [[nodiscard]] constexpr const Ok& expect_ok(StringView error) const noexcept {
    panic_if(is_error(), error);
    return ok_;
  }
  template <typename... Args>
  [[nodiscard]] constexpr const Ok& expect_ok(FormatString<Args...> error_fmt,
                                              Args&&... error_fmt_args) const noexcept {
    panic_if(is_error(), error_fmt, base::forward<Args>(error_fmt_args)...);
    return ok_;
  }
  [[nodiscard]] constexpr const Ok& expect_ok() const noexcept {
    return expect_ok("Result is not ok.");
  }

  [[nodiscard]] constexpr const Ok& assert_ok(StringView error) const noexcept {
    debug_panic_if(is_error(), error);
    return ok_;
  }
  template <typename... Args>
  [[nodiscard]] constexpr const Ok& assert_ok(FormatString<Args...> error_fmt,
                                              Args&&... error_fmt_args) const noexcept {
    debug_panic_if(is_error(), error_fmt, base::forward<Args>(error_fmt_args)...);
    return ok_;
  }
  [[nodiscard]] constexpr const Ok& assert_ok() const noexcept {
    return assert_ok("Result is not ok.");
  }

private:
  union {
    Ok ok_;
    u8 dummy_ = 0;
  };
  Error<Err> status_ = NIL;
};
// NOLINTEND(cppcoreguidelines-pro-type-union-access)

} // namespace base
