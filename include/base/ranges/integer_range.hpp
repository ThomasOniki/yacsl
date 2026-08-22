#pragma once

#include "base/type_traits/categories.hpp"

namespace base {

template <Integral T, bool Reversed = false> class IntegerRange {
public:
  class Iterator {
  public:
    explicit constexpr Iterator(T value) noexcept : value_(value) {}

    [[nodiscard]] constexpr T operator*() const noexcept { return value_; }

    constexpr T operator++() noexcept { return Reversed ? --value_ : ++value_; }
    constexpr T operator--() noexcept { return Reversed ? ++value_ : --value_; }
    constexpr T operator++(int) noexcept { return Reversed ? value_-- : value_++; }
    constexpr T operator--(int) noexcept { return Reversed ? value_++ : value_--; }

    [[nodiscard]] constexpr bool operator<=>(Iterator const& other) const noexcept = default;

    [[nodiscard]] constexpr   operator T() noexcept { return value_; }
    [[nodiscard]] constexpr T operator-(Iterator const& other) const noexcept {
      return value_ - other.value_;
    }

  private:
    T value_;
  };

  constexpr IntegerRange() noexcept = default;
  constexpr IntegerRange(T begin, T end) noexcept : begin_(begin), end_(end) {}

  [[nodiscard]] constexpr Iterator begin() const noexcept { return begin_; }
  [[nodiscard]] constexpr Iterator end() const noexcept { return end_; }

  [[nodiscard]] constexpr IntegerRange<T, true> range() const noexcept {
    return Reversed ? IntegerRange<T, false>{end_ - 1, begin_ + 1} : *this;
  }
  [[nodiscard]] constexpr IntegerRange<T, true> reversed_range() const noexcept {
    return Reversed ? *this : IntegerRange<T, true>{end_ - 1, begin_ + 1};
  }

private:
  T begin_ = 0;
  T end_   = 0;
};

} // namespace base
