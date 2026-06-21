export module base.containers.nttp_string;

import base.utilities.type_aliases;
import base.ranges.reverse_iterator;

export namespace base {

template <usize N> class NTTPString {
public:
  // NOLINTNEXTLINE(modernize-avoid-c-arrays, google-explicit-constructor)
  constexpr NTTPString(const char (&str)[N]) {
    for (usize i = 0; i < N; ++i) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
      data_[i] = str[i];
    }
  }

  [[nodiscard]] usize       size() const noexcept { return N - 1; }
  [[nodiscard]] const char* data() const noexcept { return data_; }

  [[nodiscard]] const char& operator[](usize index) { return data_[index]; }

  [[nodiscard]] const char* begin() const noexcept { return data_; }
  [[nodiscard]] const char* end() const noexcept { return data_ + N - 1; }
  [[nodiscard]] const char* cbegin() const noexcept { return data_; }
  [[nodiscard]] const char* cend() const noexcept { return data_ + N - 1; }

  [[nodiscard]] ReverseIterator<const char*> rbegin() const noexcept { return data_ + N - 1; }
  [[nodiscard]] ReverseIterator<const char*> rend() const noexcept { return data_; }
  [[nodiscard]] ReverseIterator<const char*> crbegin() const noexcept { return data_ + N - 1; }
  [[nodiscard]] ReverseIterator<const char*> crend() const noexcept { return data_; }

public:
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  char data_[N] = {'\0'};
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
template <usize N> NTTPString(const char (&str)[N]) -> NTTPString<N>;

} // namespace base
