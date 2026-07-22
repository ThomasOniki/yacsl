#pragma once

#include "base/containers/stack_storage.hpp"
#include "base/formatting/format_string.hpp"
#include "base/logging/panic.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/string_view.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/type_traits/type_string.hpp"
#include "base/utilities/nil.hpp"

namespace base {

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
template <typename T> class Option {
public:
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Option(Nil /*nil*/) noexcept : is_value_(false) {}

  template <typename... Args> // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Option(Args&&... args) noexcept : is_value_(true) {
    value_.construct(base::forward<Args>(args)...);
  }

  constexpr ~Option() noexcept {
    if (is_value_) {
      value_.destruct();
    }
  }

  constexpr Option(const Option& other) noexcept : is_value_(other.is_value_) {
    if (is_value_) {
      value_.construct(*other.value_);
    }
  }
  constexpr Option(Option&& other) noexcept : is_value_(other.is_value_) {
    if (is_value_) {
      value_.construct(base::move(*other.value_));
    }
  }
  constexpr Option& operator=(const Option& other) noexcept {
    if (&other == this) [[unlikely]] {
      return *this;
    }
    if (is_value_) {
      value_.destruct();
    }
    is_value_ = other.is_value_;
    if (is_value_) {
      value_.construct(*other.value_);
    }
    return *this;
  }
  constexpr Option& operator=(Option&& other) noexcept {
    if (is_value_) {
      value_.destruct();
    }
    is_value_ = other.is_value_;
    if (is_value_) {
      value_.construct(base::move(*other.value_));
    }
    return *this;
  }

  constexpr Option& operator=(Nil /*nil*/) noexcept {
    if (is_value_) {
      value_.destruct();
      is_value_ = false;
    }
    return *this;
  }
  constexpr Option& operator=(const T& value) noexcept {
    if (is_value_) {
      value_.destruct();
    }
    is_value_ = true;
    value_.construct(value);
    return *this;
  }
  constexpr Option& operator=(T&& value) noexcept {
    if (is_value_) {
      value_.destruct();
    }
    is_value_ = true;
    value_.construct(base::move(value));
    return *this;
  }

  template <typename... Args> constexpr T& emplace(Args&&... args) {
    if (is_value_) {
      value_.destruct();
    }
    return value_.construct(base::forward<Args>(args)...);
  }

  [[nodiscard]] constexpr bool operator==(Nil /*nil*/) const noexcept { return !is_value_; }
  [[nodiscard]] constexpr bool operator!=(Nil /*nil*/) const noexcept { return is_value_; }

  [[nodiscard]] constexpr bool operator==(const T& value) const noexcept {
    return is_value_ && *value_ == value;
  }

  [[nodiscard]] constexpr bool is_nil() const noexcept { return !is_value_; }
  [[nodiscard]] constexpr bool is_value() const noexcept { return is_value_; }

  template <typename... Args>
  [[nodiscard]] constexpr T& expect_value(FormatString<Args...> fmt, Args&&... args) noexcept {
    panic_if(!is_value_, fmt, base::forward<Args>(args)...);
    return *value_;
  }
  [[nodiscard]] constexpr T& expect_value() noexcept {
    return expect_value("Option<{}> is nil.", TYPE_STRING<T>);
  }
  template <typename... Args>
  [[nodiscard]] constexpr const T& expect_value(FormatString<Args...> fmt,
                                                Args&&... args) const noexcept {
    panic_if(!is_value_, fmt, base::forward<Args>(args)...);
    return *value_;
  }
  [[nodiscard]] constexpr const T& expect_value() const noexcept {
    return expect_value("Option<{}> is nil.", TYPE_STRING<T>);
  }

  template <typename... Args>
  [[nodiscard]] constexpr T& assert_value(FormatString<Args...> fmt, Args&&... args) noexcept {
    debug_panic_if(!is_value_, fmt, base::forward<Args>(args)...);
    return *value_;
  }
  [[nodiscard]] constexpr T& assert_value() noexcept {
    return assert_value("Option<{}> is nil.", TYPE_STRING<T>);
  }
  template <typename... Args>
  [[nodiscard]] constexpr const T& assert_value(FormatString<Args...> fmt,
                                                Args&&... args) const noexcept {
    debug_panic_if(!is_value_, fmt, base::forward<Args>(args)...);
    return *value_;
  }
  [[nodiscard]] constexpr const T& assert_value() const noexcept {
    return assert_value("Option<{}> is nil.", TYPE_STRING<T>);
  }

private:
  StackStorage<T> value_;
  bool            is_value_;
};
// NOLINTEND(cppcoreguidelines-pro-type-union-access)

template <typename T> class Option<T&> {
public:
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Option(Nil /*nil*/) noexcept {}
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Option(T& value) noexcept : ptr_(&value) {}

  constexpr ~Option() noexcept                              = default;
  constexpr Option(const Option& other) noexcept            = default;
  constexpr Option(Option&& other) noexcept                 = default;
  constexpr Option& operator=(const Option& other) noexcept = default;
  constexpr Option& operator=(Option&& other) noexcept      = default;

  constexpr Option& operator=(Nil /*nil*/) noexcept {
    ptr_ = nullptr;
    return *this;
  }
  constexpr Option& operator=(T& value) noexcept {
    ptr_ = &value;
    return *this;
  }

  constexpr T& emplace(T& value) {
    ptr_ = value;
    return *ptr_;
  }

  [[nodiscard]] constexpr bool is_nil() const noexcept { return ptr_ == nullptr; }
  [[nodiscard]] constexpr bool is_value() const noexcept { return ptr_ != nullptr; }

  [[nodiscard]] constexpr bool operator==(Nil /*nil*/) const noexcept { return ptr_ == nullptr; }
  [[nodiscard]] constexpr bool operator!=(Nil /*nil*/) const noexcept { return ptr_ != nullptr; }

  [[nodiscard]] constexpr bool operator==(const T& value) const noexcept {
    return ptr_ != nullptr && *ptr_ == value;
  }

  template <typename... Args>
  [[nodiscard]] constexpr T& expect_value(FormatString<Args...> fmt, Args&&... args) noexcept {
    panic_if(ptr_ == nullptr, fmt, base::forward<Args>(args)...);
    return *ptr_;
  }
  [[nodiscard]] constexpr T& expect_value() noexcept {
    return expect_value("Option<{}> is nil.", TYPE_STRING<T>);
  }
  template <typename... Args>
  [[nodiscard]] constexpr const T& expect_value(FormatString<Args...> fmt,
                                                Args&&... args) const noexcept {
    panic_if(ptr_ == nullptr, fmt, base::forward<Args>(args)...);
    return *ptr_;
  }
  [[nodiscard]] constexpr const T& expect_value() const noexcept {
    return expect_value("Option<{}> is nil.", TYPE_STRING<T>);
  }

  template <typename... Args>
  [[nodiscard]] constexpr T& assert_value(FormatString<Args...> fmt, Args&&... args) noexcept {
    debug_panic_if(ptr_ == nullptr, fmt, base::forward<Args>(args)...);
    return *ptr_;
  }
  [[nodiscard]] constexpr T& assert_value() noexcept {
    return assert_value("Option<{}> is nil.", TYPE_STRING<T>);
  }
  template <typename... Args>
  [[nodiscard]] constexpr const T& assert_value(FormatString<Args...> fmt,
                                                Args&&... args) const noexcept {
    debug_panic_if(ptr_ == nullptr, fmt, base::forward<Args>(args)...);
    return *ptr_;
  }
  [[nodiscard]] constexpr const T& assert_value() const noexcept {
    return assert_value("Option<{}> is nil.", TYPE_STRING<T>);
  }

private:
  T* ptr_ = nullptr;
};

} // namespace base
