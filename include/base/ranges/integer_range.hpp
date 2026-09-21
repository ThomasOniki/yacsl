#pragma once

#include "base/type_traits/categories.hpp"

namespace base::ranges {

template <type_traits::Integral T, bool Reversed = false> class IntegerRange {
public:
  class Iterator {
  public:
    explicit constexpr Iterator(T value) noexcept : value_(value) {}

    [[nodiscard]] constexpr T operator*(this Iterator self) noexcept {
      return self.value_;
    }

    constexpr T operator++(this Iterator& self) noexcept {
      return Reversed ? --self.value_ : ++self.value_;
    }
    constexpr T operator--(this Iterator& self) noexcept {
      return Reversed ? ++self.value_ : --self.value_;
    }
    constexpr T operator++(this Iterator& self, int) noexcept {
      return Reversed ? self.value_-- : self.value_++;
    }
    constexpr T operator--(this Iterator& self, int) noexcept {
      return Reversed ? self.value_++ : self.value_--;
    }

    [[nodiscard]] constexpr bool operator<=>(this Iterator self,
                                             Iterator other) noexcept = default;

    [[nodiscard]] constexpr operator T(this Iterator self) noexcept {
      return self.value_;
    }
    [[nodiscard]] constexpr T operator-(this Iterator self,
                                        Iterator      other) noexcept {
      return self.value_ - other.value_;
    }

  private:
    T value_;
  };

  constexpr IntegerRange() noexcept = default;
  constexpr IntegerRange(T begin, T end) noexcept : begin_(begin), end_(end) {}

  [[nodiscard]] constexpr auto begin(this IntegerRange self) noexcept {
    return Iterator(self.begin_);
  }
  [[nodiscard]] constexpr auto end(this IntegerRange self) noexcept {
    return Iterator(self.end_);
  }

  [[nodiscard]] constexpr IntegerRange<T, true>
  range(this IntegerRange self) noexcept {
    return Reversed ? IntegerRange<T, false>{self.end_ - 1, self.begin_ + 1}
                    : self;
  }
  [[nodiscard]] constexpr IntegerRange<T, true>
  reversed_range(this IntegerRange self) noexcept {
    return Reversed ? self
                    : IntegerRange<T, true>{self.end_ - 1, self.begin_ + 1};
  }

private:
  T begin_ = 0;
  T end_   = 0;
};

} // namespace base::ranges
