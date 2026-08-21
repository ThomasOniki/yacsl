#pragma once

namespace base {

template <typename It> class ReverseIterator {
public:
  constexpr ReverseIterator(It const& iter) noexcept : it_(iter) {}

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
  [[nodiscard]] constexpr bool operator==(ReverseIterator const& other) const noexcept {
    return it_ == other.it_;
  }
  [[nodiscard]] constexpr bool operator!=(ReverseIterator const& other) const noexcept {
    return it_ != other.it_;
  }

private:
  It it_;
};

} // namespace base
