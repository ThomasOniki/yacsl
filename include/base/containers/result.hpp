#pragma once

#include "base/attributes.hpp"
#include "base/debug/panic.hpp"
#include "base/format/format_string.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/type_traits/type_string.hpp"

namespace base::containers {

template <typename Ok, type_traits::Enum Err> class Result {
public:
  template <type_traits::MaybeCVRef<Err> E>
  Result(E&& err) : err_(memory::forward<E>(err)), is_err_(true) {}
  template <typename... Args>
  Result(Args&&... args) : ok_(memory::forward<Args>(args)...) {}

  ~Result() noexcept { destroy(); }

  constexpr Result(Result const& other) noexcept : is_err_(other.is_err_) {
    if (other.is_err_) [[unlikely]] {
      memory::construct_at(&err_, other.err_);
    } else {
      memory::construct_at(&ok_, other.ok_);
    }
  }
  constexpr Result(Result&& other) noexcept : is_err_(other.is_err_) {
    if (other.is_err_) [[unlikely]] {
      memory::construct_at(&err_, memory::move(other.err_));
    } else {
      memory::construct_at(&ok_, memory::move(other.ok_));
    }
  }

  constexpr Result& operator=(this Result& self BASE_LIFETIMEBOUND,
                              Result const&     other) noexcept {
    if (&other == &self) [[unlikely]] {
      return self;
    }
    self.destroy();
    self.is_err_ = other.is_err_;
    if (other.is_err_) [[unlikely]] {
      memory::construct_at(&self.err_, other.err_);
    } else {
      memory::construct_at(&self.ok_, other.ok_);
    }
    return self;
  }

  constexpr Result& operator=(this Result& self BASE_LIFETIMEBOUND,
                              Result&&          other) noexcept {
    self.destroy();
    self.is_err_ = other.is_err_;
    if (other.is_err_) [[unlikely]] {
      memory::construct_at(&self.err_, memory::move(other.err_));
    } else {
      memory::construct_at(&self.ok_, memory::move(other.ok_));
    }
    return self;
  }

  [[nodiscard]] constexpr bool is_ok(this Result const& self) noexcept {
    return !self.is_err_;
  }
  [[nodiscard]] constexpr bool is_err(this Result const& self) noexcept {
    return self.is_err_;
  }

  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&&
  expect_ok(this Self&& self                     BASE_LIFETIMEBOUND,
            format::FormatString<Args...> const& error_fmt,
            Args&&... error_fmt_args) noexcept {
    if (self.is_err_) [[unlikely]] {
      debug::panic("Result<{}, {}> is error {}.", type_traits::TYPE_STRING<Ok>,
                   type_traits::TYPE_STRING<Err>, self.err_);
      debug::panic(error_fmt, memory::forward<Args>(error_fmt_args)...);
    }
    return memory::forward<Self>(self).ok_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  expect_ok(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    if (self.is_err_) [[unlikely]] {
      debug::panic("Result<{}, {}> is error {}.", type_traits::TYPE_STRING<Ok>,
                   type_traits::TYPE_STRING<Err>, self.err_);
    }
    return memory::forward<Self>(self).ok_;
  }

  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&&
  assert_ok(this Self&& self                     BASE_LIFETIMEBOUND,
            format::FormatString<Args...> const& error_fmt,
            Args&&... error_fmt_args) noexcept {
    if (self.is_err_) [[unlikely]] {
      debug::debug_panic("Result<{}, {}> is error {}.",
                         type_traits::TYPE_STRING<Ok>,
                         type_traits::TYPE_STRING<Err>, self.err_);
      debug::debug_panic(error_fmt, memory::forward<Args>(error_fmt_args)...);
    }
    return memory::forward<Self>(self).ok_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  assert_ok(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    if (self.is_err_) [[unlikely]] {
      debug::debug_panic("Result<{}, {}> is error {}.",
                         type_traits::TYPE_STRING<Ok>,
                         type_traits::TYPE_STRING<Err>, self.err_);
    }
    return memory::forward<Self>(self).ok_;
  }

  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&&
  expect_err(this Self&& self                     BASE_LIFETIMEBOUND,
             format::FormatString<Args...> const& error_fmt,
             Args&&... error_fmt_args) noexcept {
    if (!self.is_err_) [[unlikely]] {
      debug::panic("Result<{}, {}> is ok {}.", type_traits::TYPE_STRING<Ok>,
                   type_traits::TYPE_STRING<Err>, self.ok_);
      debug::panic(error_fmt, memory::forward<Args>(error_fmt_args)...);
    }
    return memory::forward<Self>(self).err_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  expect_err(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    if (!self.is_err_) [[unlikely]] {
      debug::panic("Result<{}, {}> is ok {}.", type_traits::TYPE_STRING<Ok>,
                   type_traits::TYPE_STRING<Err>, self.ok_);
    }
    return memory::forward<Self>(self).err_;
  }

  template <typename Self, typename... Args>
  [[nodiscard]] constexpr auto&&
  assert_err(this Self&& self                     BASE_LIFETIMEBOUND,
             format::FormatString<Args...> const& error_fmt,
             Args&&... error_fmt_args) noexcept {
    if (!self.is_err_) [[unlikely]] {
      debug::debug_panic("Result<{}, {}> is ok {}.",
                         type_traits::TYPE_STRING<Ok>,
                         type_traits::TYPE_STRING<Err>, self.ok_);
      debug::debug_panic(error_fmt, memory::forward<Args>(error_fmt_args)...);
    }
    return memory::forward<Self>(self).err_;
  }
  template <typename Self>
  [[nodiscard]] constexpr auto&&
  assert_err(this Self&& self BASE_LIFETIMEBOUND) noexcept {
    if (!self.is_err_) [[unlikely]] {
      debug::debug_panic("Result<{}, {}> is ok {}.",
                         type_traits::TYPE_STRING<Ok>,
                         type_traits::TYPE_STRING<Err>, self.ok_);
    }
    return memory::forward<Self>(self).err_;
  }

private:
  template <typename Self> void destroy(this Self&& self) noexcept {
    if (self.is_err_) [[unlikely]] {
      memory::forward<Self>(self).err_.~Err();
    } else if constexpr (!type_traits::TriviallyDestructible<Ok>) {
      memory::forward<Self>(self).ok_.~Ok();
    }
  }

private:
  union {
    Ok  ok_;
    Err err_;
  };
  bool is_err_ = false;
};

} // namespace base::containers
