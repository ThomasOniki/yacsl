#pragma once

#include "base/attributes.hpp"
#include "base/containers/stack_array.hpp"
#include "base/memory/lifetimes.hpp"
#include "base/ranges/integer_range.hpp"
#include "base/ranges/slice.hpp"
#include "base/type_traits/categories.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

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
  constexpr void construct_all(T const& value) {
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
    return reinterpret_cast<T*>(data_.data());
  }
  [[nodiscard]] constexpr T const* data() const noexcept BASE_LIFETIMEBOUND { return data(); }

  [[nodiscard]] constexpr SizeT size() const noexcept { return static_cast<SizeT>(N); }

  [[nodiscard]] constexpr T& operator[](SizeT index) noexcept BASE_LIFETIMEBOUND {
    return data()[static_cast<usize>(index)];
  }
  [[nodiscard]] constexpr T const& operator[](SizeT index) const noexcept BASE_LIFETIMEBOUND {
    return data()[static_cast<usize>(index)];
  }

  [[nodiscard]] constexpr Slice<T, SizeT> slice() noexcept BASE_LIFETIMEBOUND {
    return {data_, static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr Slice<T const, SizeT> slice() const noexcept BASE_LIFETIMEBOUND {
    return {data_, static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr ReversedSlice<T, SizeT> reversed_slice() noexcept BASE_LIFETIMEBOUND {
    return {data_, static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr ReversedSlice<T const, SizeT>
  reversed_slice() const noexcept BASE_LIFETIMEBOUND {
    return {data_, static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept BASE_LIFETIMEBOUND {
    return {reinterpret_cast<uchar*>(data_), static_cast<SizeT>(N * sizeof(T))};
  }
  [[nodiscard]] constexpr Slice<uchar const, SizeT> bytes() const noexcept BASE_LIFETIMEBOUND {
    return {reinterpret_cast<uchar const*>(data_), static_cast<SizeT>(N * sizeof(T))};
  }

  [[nodiscard]] constexpr IntegerRange<SizeT> indices() const noexcept { return {0, N}; }

  [[nodiscard]] constexpr T*       begin() noexcept BASE_LIFETIMEBOUND { return data(); }
  [[nodiscard]] constexpr T*       end() noexcept BASE_LIFETIMEBOUND { return data() + N; }
  [[nodiscard]] constexpr T const* begin() const noexcept BASE_LIFETIMEBOUND { return data(); }
  [[nodiscard]] constexpr T const* end() const noexcept BASE_LIFETIMEBOUND { return data() + N; }

private:
  alignas(T) StackArray<uchar, N * sizeof(T)> data_;
};

} // namespace base
