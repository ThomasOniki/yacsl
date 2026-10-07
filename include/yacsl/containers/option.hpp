#pragma once

#include "yacsl/containers/nttp_string.hpp"
#include "yacsl/containers/stack_storage.hpp"
#include "yacsl/debug/_extern_functions.hpp"
#include "yacsl/debug/source_location.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/conditional.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/meta/type_string.hpp"
#include "yacsl/utils/nil.hpp"

namespace yacsl::containers {

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

  constexpr Option& operator=(this Option& self YACSL_LIFETIMEBOUND,
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
  constexpr Option& operator=(this Option& self YACSL_LIFETIMEBOUND,
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

  constexpr Option& operator=(this Option& self YACSL_LIFETIMEBOUND,
                              utils::Nil /*nil*/) noexcept {
    if (self.is_value_) {
      self.value_.destruct();
      self.is_value_ = false;
    }
    return self;
  }
  constexpr Option& operator=(this Option& self YACSL_LIFETIMEBOUND,
                              T const&          value) noexcept {
    if (self.is_value_) {
      self.value_.destruct();
    }
    self.is_value_ = true;
    self.value_.construct(value);
    return self;
  }
  constexpr Option& operator=(this Option& self YACSL_LIFETIMEBOUND,
                              T&&               value) noexcept {
    if (self.is_value_) {
      self.value_.destruct();
    }
    self.is_value_ = true;
    self.value_.construct(memory::move(value));
    return self;
  }

  template <typename Self, typename... Args>
  constexpr auto&& emplace(this Self&& self YACSL_LIFETIMEBOUND,
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

  template <typename Self>
  [[nodiscard]] constexpr auto&&
  value(this Self&& self, debug::SourceLocation location =
                              debug::SourceLocation::current()) noexcept {
    if (!self.is_value_) [[unlikely]] {
      debug::detail::extern_panic(
          cat_nttpstrings<"Option<",
                          NTTPString<meta::TYPE_STRING<T>.size()>(
                              meta::TYPE_STRING<T>),
                          "> is nil.">.view(),
          location);
    }
    return *memory::forward<Self>(self).value_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& value_unchecked(this Self&& self) noexcept {
#ifdef NDEBUG
    if (!self.is_value_) [[unlikely]] {
      __builtin_unreachable();
    }
    return *memory::forward<Self>(self).value_;
#else
    return memory::forward<Self>(self).value();
#endif
  }

  template <typename Self, typename F>
  auto transform(this Self&& self, F&& func)
      -> Option<decltype(func(*self.value_))> {
    if (self.is_value_) {
      return memory::forward<F>(func)(*memory::forward<Self>(self).value_);
    }
    return utils::NIL;
  }

private:
  StackStorage<T> value_;
  bool            is_value_;
};

template <meta::ReferenceType T> class Option<T> {
public:
  constexpr Option(utils::Nil /*nil*/) noexcept {}

  constexpr Option(T value) noexcept : ptr_(&value) {}

  constexpr ~Option() noexcept                                      = default;
  constexpr Option(Option const&) noexcept                          = default;
  constexpr Option(Option&&) noexcept                               = default;
  constexpr Option& operator=(this Option&, Option const&) noexcept = default;
  constexpr Option& operator=(this Option&, Option&&) noexcept      = default;

  constexpr Option& operator=(this Option& self YACSL_LIFETIMEBOUND,
                              utils::Nil /*nil*/) noexcept {
    self.ptr_ = nullptr;
    return self;
  }
  constexpr Option& operator=(this Option& self YACSL_LIFETIMEBOUND,
                              T                 value) noexcept {
    self.ptr_ = &value;
    return self;
  }

  constexpr T emplace(this Option& self YACSL_LIFETIMEBOUND, T value) {
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
  [[nodiscard]] constexpr bool operator==(
      this Option const&                                     self,
      meta::CopyReference<T, meta::RemoveReference<T> const> value) noexcept {
    return self.ptr_ != nullptr && self.ptr_ == &value;
  }

private:
  template <typename Self>
  using Forwarded = meta::Conditional<meta::LValueReferenceType<Self&&>, T,
                                      meta::RemoveReference<T>&&>;

public:
  template <typename Self>
  [[nodiscard]] constexpr Forwarded<Self>
  value(this Self&& self, debug::SourceLocation location =
                              debug::SourceLocation::current()) noexcept {
    if (!self.is_value_) [[unlikely]] {
      debug::detail::extern_panic(
          cat_nttpstrings<"Option<",
                          NTTPString<meta::TYPE_STRING<T>.size()>(
                              meta::TYPE_STRING<T>),
                          "> is nil.">.view(),
          location);
    }
    return *memory::forward<Self>(self).ptr_;
  }
  template <typename Self>
  [[nodiscard]] constexpr Forwarded<Self>
  value_unchecked(this Self&& self) noexcept {
#ifdef NDEBUG
    if (!self.is_value_) [[unlikely]] {
      __builtin_unreachable();
    }
    return *memory::forward<Self>(self).ptr_;
#else
    return memory::forward<Self>(self).value();
#endif
  }

private:
  meta::RemoveReference<T>* ptr_ = nullptr;
};

} // namespace yacsl::containers
