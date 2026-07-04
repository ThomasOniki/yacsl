export module base.containers.nttp_string;

import base.utilities.type_aliases;
import base.ranges.reverse_iterator;
import base.ranges.string_view;

export namespace base {

template <usize N> class NTTPString {
public:
  // NOLINTNEXTLINE(modernize-avoid-c-arrays, google-explicit-constructor)
  constexpr NTTPString(const char (&str)[N]) noexcept {
    for (usize i = 0; i < N; ++i) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
      data_[i] = str[i];
    }
  }

  // NOLINTNEXTLINE(modernize-avoid-c-arrays, google-explicit-constructor)
  constexpr NTTPString(StringView view) noexcept {
    for (usize i = 0; i < view.size(); ++i) {
      data_[i] = view[i];
    }
  }

  constexpr NTTPString() noexcept = default;

  [[nodiscard]] constexpr usize       size() const noexcept { return N - 1; }
  [[nodiscard]] constexpr char*       data() noexcept { return data_; }
  [[nodiscard]] constexpr const char* data() const noexcept { return data_; }

  [[nodiscard]] constexpr char&       operator[](usize index) noexcept { return data_[index]; }
  [[nodiscard]] constexpr const char& operator[](usize index) const noexcept {
    return data_[index];
  }

  [[nodiscard]] constexpr char*       begin() noexcept { return data_; }
  [[nodiscard]] constexpr char*       end() noexcept { return data_ + N - 1; }
  [[nodiscard]] constexpr const char* begin() const noexcept { return data_; }
  [[nodiscard]] constexpr const char* end() const noexcept { return data_ + N - 1; }
  [[nodiscard]] constexpr const char* cbegin() const noexcept { return data_; }
  [[nodiscard]] constexpr const char* cend() const noexcept { return data_ + N - 1; }

  [[nodiscard]] constexpr ReverseIterator<char*>       rbegin() noexcept { return data_ + N - 1; }
  [[nodiscard]] constexpr ReverseIterator<char*>       rend() noexcept { return data_; }
  [[nodiscard]] constexpr ReverseIterator<const char*> rbegin() const noexcept {
    return data_ + N - 1;
  }
  [[nodiscard]] constexpr ReverseIterator<const char*> rend() const noexcept { return data_; }

  [[nodiscard]] constexpr ReverseIterator<const char*> crbegin() const noexcept {
    return data_ + N - 1;
  }
  [[nodiscard]] constexpr ReverseIterator<const char*> crend() const noexcept { return data_; }

public:
  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  char data_[N] = {'\0'};
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
template <usize N> NTTPString(const char (&str)[N]) -> NTTPString<N>;

} // namespace base
