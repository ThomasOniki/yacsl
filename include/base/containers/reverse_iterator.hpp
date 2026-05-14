#pragma once

namespace base {

template <typename It> class ReverseIterator {
public:
  // NOLINTNEXTLINE(google-explicit-constructor)
  ReverseIterator(const It& iter) noexcept : it_(iter) { --it_; }
  [[nodiscard]] auto operator*() const noexcept { return *it_; }
  ReverseIterator&   operator++() noexcept;
  ReverseIterator&   operator--() noexcept;
  ReverseIterator    operator++(int) noexcept { return it_--; }
  ReverseIterator    operator--(int) noexcept { return it_++; }
  [[nodiscard]] bool operator==(const ReverseIterator& other) const noexcept {
    return it_ == other.it_;
  }
  [[nodiscard]] bool operator!=(const ReverseIterator& other) const noexcept {
    return it_ != other.it_;
  }

private:
  It it_;
};

template <typename It> ReverseIterator<It>& ReverseIterator<It>::operator++() noexcept {
  --it_;
  return *this;
}

template <typename It> ReverseIterator<It>& ReverseIterator<It>::operator--() noexcept {
  ++it_;
  return *this;
}

} // namespace base
