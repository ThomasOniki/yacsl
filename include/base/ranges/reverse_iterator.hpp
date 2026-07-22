#pragma once

namespace base {

template <typename It> class ReverseIterator {
public:
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr ReverseIterator(const It& iter) noexcept : it_(iter) {}

  [[nodiscard]] constexpr auto operator*() const noexcept {
    It tmp = it_;
    --tmp;
    return *tmp;
  }

  constexpr ReverseIterator& operator++() noexcept {
    --it_;
    return *this;
  }
  constexpr ReverseIterator& operator--() noexcept {
    ++it_;
    return *this;
  }
  constexpr ReverseIterator    operator++(int) noexcept { return it_--; }
  constexpr ReverseIterator    operator--(int) noexcept { return it_++; }
  [[nodiscard]] constexpr bool operator==(const ReverseIterator& other) const noexcept {
    return it_ == other.it_;
  }
  [[nodiscard]] constexpr bool operator!=(const ReverseIterator& other) const noexcept {
    return it_ != other.it_;
  }

private:
  It it_;
};

} // namespace base
