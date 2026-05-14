#pragma once

#include "base/containers/fwds.hpp"
#include "base/containers/reverse_iterator.hpp"
#include "base/containers/stack_array.hpp"
#include "base/utilities/move_forward.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

// THIS DOES NOT KEEP TRACK OF WHICH ELEMENTS WHERE CREATED AND DESTROYED, YOU ARE RESPONSIBLE FOR
// THAT.

template <typename T, usize N, typename SizeT> class StackArrayStorage {
public:
  template <typename... Args> constexpr void construct_at(SizeT index, Args&&... args) {
    new (data() + index) T(base::forward<Args>(args)...);
  }

  constexpr void destruct_at(SizeT index) { data()[index].~T(); }

  constexpr void construct_all() {
    for (usize i = 0; i < N; ++i) {
      new (data() + i) T();
    }
  }
  constexpr void construct_all(const T& value) {
    for (usize i = 0; i < N; ++i) {
      new (data() + i) T(value);
    }
  }

  constexpr void destruct_all() {
    for (usize i = 0; i < N; ++i) {
      data()[i].~T();
    }
  }

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  [[nodiscard]] constexpr T*       data() noexcept { return reinterpret_cast<T*>(data_.data()); }
  [[nodiscard]] constexpr const T* data() const noexcept { return data(); }

  [[nodiscard]] constexpr SizeT size() const noexcept { return static_cast<SizeT>(N); }

  [[nodiscard]] constexpr T& operator[](SizeT index) noexcept {
    return data()[static_cast<usize>(index)];
  }
  [[nodiscard]] constexpr const T& operator[](SizeT index) const noexcept {
    return data()[static_cast<usize>(index)];
  }

  [[nodiscard]] constexpr T*       begin() noexcept { return data(); }
  [[nodiscard]] constexpr T*       end() noexcept { return data() + N; }
  [[nodiscard]] constexpr const T* begin() const noexcept { return data(); }
  [[nodiscard]] constexpr const T* end() const noexcept { return data() + N; }
  [[nodiscard]] constexpr const T* cbegin() const noexcept { return data(); }
  [[nodiscard]] constexpr const T* cend() const noexcept { return data() + N; }

  [[nodiscard]] constexpr ReverseIterator<T*>       rbegin() noexcept { return data() + N; }
  [[nodiscard]] constexpr ReverseIterator<T*>       rend() noexcept { return data(); }
  [[nodiscard]] constexpr ReverseIterator<const T*> rbegin() const noexcept { return data() + N; }
  [[nodiscard]] constexpr ReverseIterator<const T*> rend() const noexcept { return data(); }
  [[nodiscard]] constexpr ReverseIterator<const T*> crbegin() const noexcept { return data() + N; }
  [[nodiscard]] constexpr ReverseIterator<const T*> crend() const noexcept { return data(); }

private:
  alignas(T) StackArray<uchar, N * sizeof(T)> data_;
};

} // namespace base
