#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/containers/option.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/conditional.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/meta/type_string.hpp"
#include "yacsl/utils/nil.hpp"

namespace yacsl::containers {

template <typename T, typename E> class Result {
private:
  struct ValueTag {};
  struct ErrorTag {};

  template <typename... Args>
  Result(ValueTag /*tag*/, Args&&... args)
    requires(!meta::ReferenceType<T>)
      : value_(memory::forward<Args>(args)...) {}

  Result(ValueTag /*tag*/, T ref)
    requires meta::ReferenceType<T>
      : value_(&ref) {}

  template <typename... Args>
  Result(ErrorTag /*tag*/, Args&&... args)
    requires(!meta::ReferenceType<E>)
      : error_(memory::forward<Args>(args)...), is_error_(true) {}

  Result(ErrorTag /*tag*/, E ref)
    requires meta::ReferenceType<E>
      : error_(&ref), is_error_(true) {}

public:
  template <meta::MaybeCVRef<E> Err>
  Result(Err&& error) : Result(ErrorTag{}, memory::forward<Err>(error)) {}
  template <typename... Args>
  Result(Args&&... args) : Result(ValueTag{}, memory::forward<Args>(args)...) {}

  template <typename... Args> static Result new_error(Args&&... args) {
    return Result(ErrorTag{}, memory::forward<Args>(args)...);
  }
  template <typename... Args> static Result new_value(Args&&... args) {
    return Result(ValueTag{}, memory::forward<Args>(args)...);
  }

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
  auto value_option(this Self&& self YACSL_LIFETIMEBOUND) noexcept
      -> Option<decltype((memory::forward<Self>(self).value_))>
    requires(!meta::ReferenceType<T>)
  {
    if (self.is_error_) {
      return utils::NIL;
    }
    return memory::forward<Self>(self).value_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  value(this Self&& self YACSL_LIFETIMEBOUND) noexcept
    requires(!meta::ReferenceType<T>)
  {
    if (self.is_error_) [[unlikely]] {
      debug::panic("Result<{}, {}> is error {}.", meta::TYPE_STRING<T>,
                   meta::TYPE_STRING<E>, self.get_error());
    }
    return memory::forward<Self>(self).value_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  value_unchecked(this Self&& self YACSL_LIFETIMEBOUND) noexcept
    requires(!meta::ReferenceType<T>)
  {
    if (self.is_error_) [[unlikely]] {
      debug::unreachable("Result<{}, {}> is error {}.", meta::TYPE_STRING<T>,
                         meta::TYPE_STRING<E>, self.get_error());
    }
    return memory::forward<Self>(self).value_;
  }

  template <typename Self>
  auto value_option(this Self&& self) noexcept -> Option<T>
    requires meta::ReferenceType<T>
  {
    if (self.is_error_) {
      return utils::NIL;
    }
    return memory::forward<T>(*memory::forward<Self>(self).value_);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& value(this Self&& self) noexcept
    requires meta::ReferenceType<T>
  {
    if (self.is_error_) [[unlikely]] {
      debug::panic("Result<{}, {}> is error {}.", meta::TYPE_STRING<T>,
                   meta::TYPE_STRING<E>, self.get_error());
    }
    return memory::forward<T>(*memory::forward<Self>(self).value_);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& value_unchecked(this Self&& self) noexcept
    requires meta::ReferenceType<T>
  {
    if (self.is_error_) [[unlikely]] {
      debug::unreachable("Result<{}, {}> is error {}.", meta::TYPE_STRING<T>,
                         meta::TYPE_STRING<E>, self.get_error());
    }
    return memory::forward<T>(*memory::forward<Self>(self).value_);
  }

  template <typename Self>
  auto error_option(this Self&& self) noexcept
      -> Option<decltype((memory::forward<Self>(self).error_))>
    requires(!meta::ReferenceType<E>)
  {
    if (self.is_error_) {
      return utils::NIL;
    }
    return memory::forward<Self>(self).error_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& error(this Self&& self) noexcept
    requires(!meta::ReferenceType<E>)
  {
    if (!self.is_error_) [[unlikely]] {
      debug::panic("Result<{}, {}> is value {}.", meta::TYPE_STRING<T>,
                   meta::TYPE_STRING<E>, self.get_value());
    }
    return memory::forward<Self>(self).error_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& error_unchecked(this Self&& self) noexcept
    requires(!meta::ReferenceType<E>)
  {
    if (!self.is_error_) [[unlikely]] {
      debug::unreachable("Result<{}, {}> is value {}.", meta::TYPE_STRING<T>,
                         meta::TYPE_STRING<E>, self.get_value());
    }
    return memory::forward<Self>(self).error_;
  }

  template <typename Self>
  auto error_option(this Self&& self) noexcept -> Option<E>
    requires meta::ReferenceType<E>
  {
    if (self.is_error_) {
      return utils::NIL;
    }
    return memory::forward<Self>(self).error_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& error(this Self&& self) noexcept
    requires meta::ReferenceType<E>
  {
    if (!self.is_error_) [[unlikely]] {
      debug::panic("Result<{}, {}> is value {}.", meta::TYPE_STRING<T>,
                   meta::TYPE_STRING<E>, self.get_value());
    }
    return memory::forward<E>(*memory::forward<Self>(self).error_);
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&& error_unchecked(this Self&& self) noexcept
    requires meta::ReferenceType<E>
  {
    if (!self.is_error_) [[unlikely]] {
      debug::unreachable("Result<{}, {}> is value {}.", meta::TYPE_STRING<T>,
                         meta::TYPE_STRING<E>, self.get_value());
    }
    return memory::forward<E>(*memory::forward<Self>(self).error_);
  }

  template <typename Self, typename F>
  auto transform(this Self&& self, F&& func)
      -> Result<decltype(func(self.value())), E> {
    if (self.is_value_) {
      return memory::forward<F>(func)(memory::forward<Self>(self).get_value());
    }
    return memory::forward<E>(*memory::forward<Self>(self).error_);
  }

private:
  template <typename Self> void destroy(this Self&& self) noexcept {
    if (self.is_error_) [[unlikely]] {
      memory::forward<Self>(self).error_.~ErrorVarient();
    } else {
      memory::forward<Self>(self).value_.~ValueVarient();
    }
  }

  template <typename Self> auto&& get_value(this Self&& self) noexcept {
    if constexpr (meta::ReferenceType<T>) {
      return memory::forward<T>(*memory::forward<Self>(self).value_);
    } else {
      return memory::forward<Self>(self).value_;
    }
  }

  template <typename Self> auto&& get_error(this Self&& self) noexcept {
    if constexpr (meta::ReferenceType<E>) {
      return memory::forward<E>(*memory::forward<Self>(self).error_);
    } else {
      return memory::forward<Self>(self).error_;
    }
  }

private:
  using ValueVarient =
      meta::Conditional<meta::ReferenceType<T>, meta::RemoveReference<T>*, T>;
  using ErrorVarient =
      meta::Conditional<meta::ReferenceType<E>, meta::RemoveReference<E>*, E>;
  union {
    ValueVarient value_;
    ErrorVarient error_;
  };
  bool is_error_ = false;
};

} // namespace yacsl::containers
