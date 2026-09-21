#pragma once

#include "base/containers/stack_storage.hpp"
#include "base/debug/panic.hpp"
#include "base/format/format_string.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_traits/type_string.hpp"
#include "base/utils/nil.hpp"

namespace base::containers {

template <typename T> class Option {
public:
  constexpr Option(utils::Nil /*nil*/) noexcept : is_value_(false) {}

  template <typename... Args>
  constexpr Option(Args&&... args) noexcept : is_value_(true) {
    value_.construct(memory::forward<Args>(args)...);
  }

  constexpr ~Option() noexcept {
    if (is_value_) {
      value_.destruct();
    }
  }

  constexpr Option(Option const& other) noexcept : is_value_(other.is_value_) {
    if (is_value_) {
      value_.construct(*other.value_);
    }
  }
  constexpr Option(Option&& other) noexcept : is_value_(other.is_value_) {
    if (is_value_) {
      value_.construct(memory::move(*other.value_));
    }
  }

  constexpr Option& operator=(this Option& self BASE_LIFETIMEBOUND,
                              Option const&     other) noexcept {
    if (&other == &self) [[unlikely]] {
      return self;
    }
    if (self.is_value_) {
      self.value_.destruct();
    }
    self.is_value_ = other.is_value_;
    if (self.is_value_) {
      self.value_.construct(*other.value_);
    }
    return self;
  }
  constexpr Option& operator=(this Option& self BASE_LIFETIMEBOUND,
                              Option&&          other) noexcept {
    if (self.is_value_) {
      self.value_.destruct();
    }
    self.is_value_ = other.is_value_;
    if (self.is_value_) {
      self.value_.construct(memory::move(*other.value_));
    }
    return self;
  }

  constexpr Option& operator=(this Option& self BASE_LIFETIMEBOUND,
                              utils::Nil /*nil*/) noexcept {
    if (self.is_value_) {
      self.value_.destruct();
      self.is_value_ = false;
    }
    return self;
  }
  constexpr Option& operator=(this Option& self BASE_LIFETIMEBOUND,
                              T const&          value) noexcept {
    if (self.is_value_) {
      self.value_.destruct();
    }
    self.is_value_ = true;
    self.value_.construct(value);
    return self;
  }
  constexpr Option& operator=(this Option& self BASE_LIFETIMEBOUND,
                              T&&               value) noexcept {
    if (self.is_value_) {
      self.value_.destruct();
    }
    self.is_value_ = true;
    self.value_.construct(memory::move(value));
    return self;
  }

  template <typename Self, typename... Args>
  constexpr auto&& emplace(this Self&& self BASE_LIFETIMEBOUND,
                           Args&&... args) {
    if (self.is_value_) {
      self.value_.destruct();
    }
    return memory::forward<Self>(self).value_.construct(
        memory::forward<Args>(args)...);
  }

  [[nodiscard]] constexpr bool operator==(this Option const& self,
                                          utils::Nil /*nil*/) noexcept {
    return !self.is_value_;
  }
  [[nodiscard]] constexpr bool operator!=(this Option const& self,
                                          utils::Nil /*nil*/) noexcept {
    return self.is_value_;
  }

  [[nodiscard]] constexpr bool operator==(this Option const& self,
                                          T const&           value) noexcept {
    return self.is_value_ && *self.value_ == value;
  }

  [[nodiscard]] constexpr bool is_nil(this Option const& self) noexcept {
    return !self.is_value_;
  }
  [[nodiscard]] constexpr bool is_value(this Option const& self) noexcept {
    return self.is_value_;
  }

  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&&
  expect_value(this Self&& self                     BASE_LIFETIMEBOUND,
               format::FormatString<Args...> const& fmt,
               Args&&... args) noexcept {
    debug::panic_if(!self.is_value_, fmt, memory::forward<Args>(args)...);
    return *memory::forward<Self>(self).value_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& expect_value(this Self&& self) noexcept {
    return memory::forward<Self>(self).expect_value(
        "Option<{}> is nil.", type_traits::TYPE_STRING<T>);
  }
  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&&
  assert_value(this Self&& self                     BASE_LIFETIMEBOUND,
               format::FormatString<Args...> const& fmt,
               Args&&... args) noexcept {
    debug::debug_panic_if(!self.is_value_, fmt, memory::forward<Args>(args)...);
    return *memory::forward<Self>(self).value_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& assert_value(this Self&& self) noexcept {
    return memory::forward<Self>(self).assert_value(
        "Option<{}> is nil.", type_traits::TYPE_STRING<T>);
  }

private:
  StackStorage<T> value_;
  bool            is_value_;
};

template <typename T> class Option<T&> {
public:
  constexpr Option(utils::Nil /*nil*/) noexcept {}

  constexpr Option(T& value) noexcept : ptr_(&value) {}

  constexpr ~Option() noexcept                                      = default;
  constexpr Option(Option const&) noexcept                          = default;
  constexpr Option(Option&&) noexcept                               = default;
  constexpr Option& operator=(this Option&, Option const&) noexcept = default;
  constexpr Option& operator=(this Option&, Option&&) noexcept      = default;

  constexpr Option& operator=(this Option& self BASE_LIFETIMEBOUND,
                              utils::Nil /*nil*/) noexcept {
    self.ptr_ = nullptr;
    return self;
  }
  constexpr Option& operator=(this Option& self BASE_LIFETIMEBOUND,
                              T&                value) noexcept {
    self.ptr_ = &value;
    return self;
  }

  constexpr T& emplace(this Option& self BASE_LIFETIMEBOUND, T& value) {
    self.ptr_ = &value;
    return *self.ptr_;
  }

  [[nodiscard]] constexpr bool is_nil(this Option const& self) noexcept {
    return self.ptr_ == nullptr;
  }
  [[nodiscard]] constexpr bool is_value(this Option const& self) noexcept {
    return self.ptr_ != nullptr;
  }

  [[nodiscard]] constexpr bool operator==(this Option const& self,
                                          utils::Nil /*nil*/) noexcept {
    return self.ptr_ == nullptr;
  }
  [[nodiscard]] constexpr bool operator!=(this Option const& self,
                                          utils::Nil /*nil*/) noexcept {
    return self.ptr_ != nullptr;
  }
  [[nodiscard]] constexpr bool operator==(this Option const& self,
                                          T const&           value) noexcept {
    return self.ptr_ != nullptr && self.ptr_ == &value;
  }

  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&&
  expect_value(this Self&& self                     BASE_LIFETIMEBOUND,
               format::FormatString<Args...> const& fmt,
               Args&&... args) noexcept {
    debug::panic_if(self.ptr_ == nullptr, fmt, memory::forward<Args>(args)...);
    return memory::forward<type_traits::CopyCVRef<Self&&, T>>(
        *memory::forward<Self>(self).ptr_);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  expect_value(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).expect_value(
        "Option<{}> is nil.", type_traits::TYPE_STRING<T>);
  }

  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&&
  assert_value(this Self&& self                     BASE_LIFETIMEBOUND,
               format::FormatString<Args...> const& fmt,
               Args&&... args) noexcept {
    debug::debug_panic_if(self.ptr_ == nullptr, fmt,
                          memory::forward<Args>(args)...);
    return memory::forward<type_traits::CopyCVRef<Self&&, T>>(
        *memory::forward<Self>(self).ptr_);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  assert_value(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).assert_value(
        "Option<{}> is nil.", type_traits::TYPE_STRING<T>);
  }

private:
  T* ptr_ = nullptr;
};

} // namespace base::containers
