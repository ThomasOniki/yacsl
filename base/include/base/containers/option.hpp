#pragma once

#include "base/containers/string_view.hpp"
#include "base/logging/common.hpp"
#include "base/logging/errors.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/nil.hpp"

namespace base {

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
template <typename T> class Option {
public:
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Option(Nil /*nil*/) noexcept : is_value_(false) {}

  template <typename... Args> // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Option(Args&&... args) noexcept : value_(base::forward<Args>(args)...) {}

  constexpr ~Option() noexcept {
    if constexpr (!TriviallyDestructible<T>) {
      if (is_value_) {
        value_.~T();
      }
    }
  }

  constexpr Option(const Option& other) noexcept : is_value_(other.is_value_) {
    if (is_value_) {
      new (&value_) T(other.value_);
    }
  }
  constexpr Option(Option&& other) noexcept : is_value_(other.is_value_) {
    if (is_value_) {
      new (&value_) T(base::move(other.value_));
    }
  }
  constexpr Option& operator=(const Option& other) noexcept {
    if (&other == this) [[unlikely]] {
      return *this;
    }
    if constexpr (!TriviallyDestructible<T>) {
      if (is_value_) {
        value_.~T();
      }
    }
    is_value_ = other.is_value_;
    if (is_value_) {
      value_ = other.value_;
    }
    return *this;
  }
  constexpr Option& operator=(Option&& other) noexcept {
    if constexpr (!TriviallyDestructible<T>) {
      if (is_value_) {
        value_.~T();
      }
    }
    is_value_ = other.is_value_;
    if (is_value_) {
      value_ = base::move(other.value_);
    }
    return *this;
  }

  [[nodiscard]] constexpr bool operator==(Nil /*nil*/) const noexcept { return !is_value_; }
  [[nodiscard]] constexpr bool operator!=(Nil /*nil*/) const noexcept { return is_value_; }

  [[nodiscard]] constexpr bool is_nil() const noexcept { return !is_value_; }
  [[nodiscard]] constexpr bool is_value() const noexcept { return is_value_; }

  [[nodiscard]] constexpr const T& expect_value(StringView error) const noexcept {
    panic_if(!is_value_, error);
    return value_;
  }
  template <typename... Args>
  [[nodiscard]] constexpr const T& expect_value(FormatString<Args...> fmt,
                                                Args&&... args) const noexcept {
    panic_if(!is_value_, fmt, base::forward<Args>(args)...);
    return value_;
  }
  [[nodiscard]] constexpr const T& expect_value() const noexcept {
    return expect_value("Optional has no value.");
  }

  [[nodiscard]] constexpr const T& assert_value(StringView error) const noexcept {
    debug_panic_if(!is_value_, error);
    return value_;
  }
  template <typename... Args>
  [[nodiscard]] constexpr const T& assert_value(FormatString<Args...> fmt,
                                                Args&&... args) const noexcept {
    debug_panic_if(!is_value_, fmt, base::forward<Args>(args)...);
    return value_;
  }
  [[nodiscard]] constexpr const T& assert_value() const noexcept {
    return assert_value("Optional has no value.");
  }

private:
  union {
    T   value_;
    Nil dummy_{};
  };
  bool is_value_;
};
// NOLINTEND(cppcoreguidelines-pro-type-union-access)

} // namespace base
