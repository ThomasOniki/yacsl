export module base.ranges.slice;

import base.type_traits.categories;
import base.type_traits.qualifiers;
import base.type_traits.comparaison;
import base.utilities.type_aliases;
import base.ranges.reverse_iterator;

export namespace base {

template <typename T, Integral SizeT = usize> class Slice {
public:
  constexpr Slice(T* begin, T* end) noexcept
      : data_(begin), size_(static_cast<SizeT>(end - begin)) {}

  constexpr Slice(T* data, SizeT size) noexcept : data_(data), size_(size) {}

  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Slice(const Slice<RemoveConst<T>>& mutable_slice) noexcept
      : data_(mutable_slice.data_), size_(mutable_slice.size_) {}

  [[nodiscard]] constexpr T*       data() noexcept { return data_; }
  [[nodiscard]] constexpr const T* data() const noexcept { return data_; }
  [[nodiscard]] constexpr SizeT    size() const noexcept { return size_; }

  [[nodiscard]] constexpr T*       begin() noexcept { return data_; }
  [[nodiscard]] constexpr T*       end() noexcept { return data_ + size_; }
  [[nodiscard]] constexpr const T* begin() const noexcept { return data_; }
  [[nodiscard]] constexpr const T* end() const noexcept { return data_ + size_; }
  [[nodiscard]] constexpr const T* cbegin() const noexcept { return data_; }
  [[nodiscard]] constexpr const T* cend() const noexcept { return data_ + size_; }

  [[nodiscard]] constexpr ReverseIterator<T*>       rbegin() noexcept { return data_ + size_; }
  [[nodiscard]] constexpr ReverseIterator<T*>       rend() noexcept { return data_; }
  [[nodiscard]] constexpr ReverseIterator<const T*> rbegin() const noexcept {
    return data_ + size_;
  }
  [[nodiscard]] constexpr ReverseIterator<const T*> rend() const noexcept { return data_; }
  [[nodiscard]] constexpr ReverseIterator<const T*> crbegin() const noexcept {
    return data_ + size_;
  }
  [[nodiscard]] constexpr ReverseIterator<const T*> crend() const noexcept { return data_; }

private:
  T*    data_;
  SizeT size_;
};

} // namespace base
