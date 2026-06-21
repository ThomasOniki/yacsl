export module base.containers.error;

import base.type_traits.categories;
import base.utilities.nil;
import base.memory.lifetimes;
import base.ranges.string_view;
import base.logging.format_string;
import base.logging.panic;

export namespace base {

template <Enum Err> class Error {
public:
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Error(Nil /*nil*/) noexcept {}
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Error(Err error) noexcept : value_(error) {}

  [[nodiscard]] constexpr bool operator==(Err error) const noexcept { return value_ == error; }
  [[nodiscard]] constexpr bool operator==(Nil /*nil*/) const noexcept { return value_ == Err{0}; }

  [[nodiscard]] constexpr Err error() const noexcept { return value_; }

  template <typename... Args>
  constexpr void expect_nil(const FormatString<Args...>& error_fmt, Args&&... args) {
    if (value_ != Err{0}) [[unlikely]] {
      panic(error_fmt, base::forward<Args>(args)...);
    }
  }
  constexpr void expect_nil() { expect_nil("Error is not nil."); }

  template <typename... Args>
  constexpr void assert_nil(const FormatString<Args...>& error_fmt, Args&&... args) {
    if (value_ != Err{0}) [[unlikely]] {
      debug_panic(error_fmt, base::forward<Args>(args)...);
    }
  }
  constexpr void assert_nil() { assert_nil("Error is not nil."); }

private:
  Err value_{0};
};

template <Enum Err> Error(Err) -> Error<Err>;

} // namespace base
