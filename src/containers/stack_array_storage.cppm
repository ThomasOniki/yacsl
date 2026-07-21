module;

#include "base/attributes.hpp"

export module base.containers.stack_array_storage;

import base.containers.stack_array;
import base.utilities.type_aliases;
import base.memory.lifetimes;
import base.type_traits.categories;
import base.ranges.reverse_iterator;
import base.ranges.slice;
import base.ranges.integer_range;

export namespace base {

// THIS DOES NOT KEEP TRACK OF WHICH ELEMENTS WHERE CREATED AND DESTROYED, YOU ARE RESPONSIBLE FOR
// THAT.

template <typename T, usize N, Integral SizeT = usize> class StackArrayStorage {
public:
  template <typename... Args> constexpr void construct_at(SizeT index, Args&&... args) {
    base::construct_at(data() + index, base::forward<Args>(args)...);
  }

  constexpr void destruct_at(SizeT index) { data()[index].~T(); }

  constexpr void construct_all() {
    for (usize i = 0; i < N; ++i) {
      base::construct_at(data() + i);
    }
  }
  constexpr void construct_all(const T& value) {
    for (usize i = 0; i < N; ++i) {
      base::construct_at(data() + i, value);
    }
  }

  constexpr void destruct_all() {
    for (usize i = 0; i < N; ++i) {
      data()[i].~T();
    }
  }

  [[nodiscard]] constexpr T* data() noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<T*>(data_.data());
  }
  [[nodiscard]] constexpr const T* data() const noexcept BASE_LIFETIMEBOUND { return data(); }

  [[nodiscard]] constexpr SizeT size() const noexcept { return static_cast<SizeT>(N); }

  [[nodiscard]] constexpr T& operator[](SizeT index) noexcept BASE_LIFETIMEBOUND {
    return data()[static_cast<usize>(index)];
  }
  [[nodiscard]] constexpr const T& operator[](SizeT index) const noexcept BASE_LIFETIMEBOUND {
    return data()[static_cast<usize>(index)];
  }

  [[nodiscard]] constexpr Slice<T, SizeT> slice() noexcept BASE_LIFETIMEBOUND {
    return {data_, static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr Slice<const T, SizeT> slice() const noexcept BASE_LIFETIMEBOUND {
    return {data_, static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<uchar*>(data_), static_cast<SizeT>(N * sizeof(T))};
  }
  [[nodiscard]] constexpr Slice<const uchar, SizeT> bytes() const noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const uchar*>(data_), static_cast<SizeT>(N * sizeof(T))};
  }

  [[nodiscard]] constexpr IntegerRange<SizeT> indices() const noexcept { return {0, N}; }

  [[nodiscard]] constexpr T*       begin() noexcept BASE_LIFETIMEBOUND { return data(); }
  [[nodiscard]] constexpr T*       end() noexcept BASE_LIFETIMEBOUND { return data() + N; }
  [[nodiscard]] constexpr const T* begin() const noexcept BASE_LIFETIMEBOUND { return data(); }
  [[nodiscard]] constexpr const T* end() const noexcept BASE_LIFETIMEBOUND { return data() + N; }
  [[nodiscard]] constexpr const T* cbegin() const noexcept BASE_LIFETIMEBOUND { return data(); }
  [[nodiscard]] constexpr const T* cend() const noexcept BASE_LIFETIMEBOUND { return data() + N; }

  [[nodiscard]] constexpr ReverseIterator<T*> rbegin() noexcept BASE_LIFETIMEBOUND {
    return data() + N;
  }
  [[nodiscard]] constexpr ReverseIterator<T*> rend() noexcept BASE_LIFETIMEBOUND { return data(); }
  [[nodiscard]] constexpr ReverseIterator<const T*> rbegin() const noexcept BASE_LIFETIMEBOUND {
    return data() + N;
  }
  [[nodiscard]] constexpr ReverseIterator<const T*> rend() const noexcept BASE_LIFETIMEBOUND {
    return data();
  }
  [[nodiscard]] constexpr ReverseIterator<const T*> crbegin() const noexcept BASE_LIFETIMEBOUND {
    return data() + N;
  }
  [[nodiscard]] constexpr ReverseIterator<const T*> crend() const noexcept BASE_LIFETIMEBOUND {
    return data();
  }

private:
  alignas(T) StackArray<uchar, N * sizeof(T)> data_;
};

} // namespace base
