#pragma once

#include "base/type_traits/categories.hpp"

namespace base {

template <Integral T> class IntegerRange {
public:
  class Iterator {
  public:
    explicit constexpr Iterator(T value) noexcept : value_(value) {}

    [[nodiscard]] constexpr T operator*() const noexcept { return value_; }

    constexpr T operator++() noexcept { return ++value_; }
    constexpr T operator--() noexcept { return --value_; }
    constexpr T operator++(int) noexcept { return value_++; }
    constexpr T operator--(int) noexcept { return value_--; }

    [[nodiscard]] constexpr bool operator<=>(const Iterator& other) const noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    [[nodiscard]] constexpr   operator T() noexcept { return value_; }
    [[nodiscard]] constexpr T operator-(const Iterator& other) const noexcept {
      return value_ - other.value_;
    }

  private:
    T value_;
  };
  class ReverseIterator {
  public:
    explicit constexpr ReverseIterator(T value) noexcept : value_(value) {}

    [[nodiscard]] constexpr T operator*() const noexcept { return value_; }

    constexpr T operator++() noexcept { return --value_; }
    constexpr T operator--() noexcept { return ++value_; }
    constexpr T operator++(int) noexcept { return value_--; }
    constexpr T operator--(int) noexcept { return value_++; }

    [[nodiscard]] constexpr bool operator<=>(const ReverseIterator& other) const = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    [[nodiscard]] constexpr   operator T() noexcept { return value_; }
    [[nodiscard]] constexpr T operator-(const ReverseIterator& other) const noexcept {
      return value_ - other.value_;
    }

  private:
    T value_;
  };

  constexpr IntegerRange() noexcept = default;
  constexpr IntegerRange(T begin, T end) noexcept : begin_(begin), end_(end) {}

  [[nodiscard]] constexpr auto begin() const noexcept { return Iterator(begin_); }
  [[nodiscard]] constexpr auto end() const noexcept { return Iterator(end_); }
  [[nodiscard]] constexpr auto cbegin() const noexcept { return Iterator(begin_); }
  [[nodiscard]] constexpr auto cend() const noexcept { return Iterator(end_); }
  [[nodiscard]] constexpr auto rbegin() const noexcept { return ReverseIterator(end_); }
  [[nodiscard]] constexpr auto rend() const noexcept { return ReverseIterator(begin_); }
  [[nodiscard]] constexpr auto crbegin() const noexcept { return ReverseIterator(end_); }
  [[nodiscard]] constexpr auto crend() const noexcept { return ReverseIterator(begin_); }

  [[nodiscard]] constexpr IntegerRange reversed_range() const noexcept { return {end_, begin_}; }

private:
  T begin_ = 0;
  T end_   = 0;
};

} // namespace base
