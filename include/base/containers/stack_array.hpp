#pragma once

#include "base/attributes.hpp"
#include "base/containers/structured_bindings.hpp"
#include "base/metaprogramming/type_manip.hpp"
#include "base/ranges/integer_range.hpp"
#include "base/ranges/reverse_iterator.hpp"
#include "base/ranges/slice.hpp"
#include "base/type_traits/categories.hpp"
#include "base/type_traits/limits.hpp"
#include "base/utilities/type_aliases.hpp"

namespace base {

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
template <typename T, usize N, Integral SizeT = ConditionalT<(N > Limits<u32>::max), usize, u32>>
class StackArray {
public:
  [[nodiscard]] constexpr SizeT size() const noexcept { return static_cast<SizeT>(N); }

  [[nodiscard]] constexpr T* data() noexcept BASE_LIFETIMEBOUND { return static_cast<T*>(data_); }
  [[nodiscard]] constexpr T const* data() const noexcept BASE_LIFETIMEBOUND {
    return static_cast<T const*>(data_);
  }

  [[nodiscard]] constexpr T& operator[](SizeT index) noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    return data_[index];
  }
  [[nodiscard]] constexpr T const& operator[](SizeT index) const noexcept BASE_LIFETIMEBOUND {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    return data_[index];
  }

  [[nodiscard]] constexpr Slice<T, SizeT> slice() noexcept BASE_LIFETIMEBOUND {
    return {data_, static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr Slice<T const, SizeT> slice() const noexcept BASE_LIFETIMEBOUND {
    return {data_, static_cast<SizeT>(N)};
  }
  [[nodiscard]] constexpr Slice<uchar, SizeT> bytes() noexcept BASE_LIFETIMEBOUND {
    return {reinterpret_cast<uchar*>(data_), static_cast<SizeT>(N * sizeof(T))};
  }
  [[nodiscard]] constexpr Slice<uchar const, SizeT> bytes() const noexcept BASE_LIFETIMEBOUND {
    return {reinterpret_cast<uchar const*>(data_), static_cast<SizeT>(N * sizeof(T))};
  }

  [[nodiscard]] constexpr IntegerRange<SizeT> indices() const noexcept { return {0, N}; }

  [[nodiscard]] constexpr T&       front() noexcept BASE_LIFETIMEBOUND { return data_[0]; }
  [[nodiscard]] constexpr T&       back() noexcept BASE_LIFETIMEBOUND { return data_[N - 1]; }
  [[nodiscard]] constexpr T const& front() const noexcept BASE_LIFETIMEBOUND { return data_[0]; }
  [[nodiscard]] constexpr T const& back() const noexcept BASE_LIFETIMEBOUND { return data_[N - 1]; }

  [[nodiscard]] constexpr T*       begin() noexcept BASE_LIFETIMEBOUND { return data_; }
  [[nodiscard]] constexpr T*       end() noexcept BASE_LIFETIMEBOUND { return data_ + N; }
  [[nodiscard]] constexpr T const* begin() const noexcept BASE_LIFETIMEBOUND { return data_; }
  [[nodiscard]] constexpr T const* end() const noexcept BASE_LIFETIMEBOUND { return data_ + N; }
  [[nodiscard]] constexpr T const* cbegin() const noexcept BASE_LIFETIMEBOUND { return data_; }
  [[nodiscard]] constexpr T const* cend() const noexcept BASE_LIFETIMEBOUND { return data_ + N; }

  [[nodiscard]] constexpr ReverseIterator<T*> rbegin() noexcept BASE_LIFETIMEBOUND { data_ + N; }
  [[nodiscard]] constexpr ReverseIterator<T*> rend() noexcept BASE_LIFETIMEBOUND { return data_; }
  [[nodiscard]] constexpr ReverseIterator<T const*> rbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_ + N;
  }
  [[nodiscard]] constexpr ReverseIterator<T const*> rend() const noexcept BASE_LIFETIMEBOUND {
    return data_;
  }
  [[nodiscard]] constexpr ReverseIterator<T const*> crbegin() const noexcept BASE_LIFETIMEBOUND {
    return data_ + N;
  }
  [[nodiscard]] constexpr ReverseIterator<T const*> crend() const noexcept BASE_LIFETIMEBOUND {
    return data_;
  }

  template <usize I> [[nodiscard]] constexpr T& get() noexcept BASE_LIFETIMEBOUND {
    return data_[I];
  }
  template <usize I> [[nodiscard]] constexpr T const& get() const noexcept BASE_LIFETIMEBOUND {
    return data_[I];
  }

  constexpr bool operator==(StackArray const& other) const noexcept = default;

public:
  T data_[N]; // NOLINT
};
// NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)

template <typename T, usize N, Integral SizeT> struct TupleSize<StackArray<T, N, SizeT>> {
  static constexpr usize value = N;
};

template <typename T, usize N, Integral SizeT, usize I>
struct TupleElement<StackArray<T, N, SizeT>, I> {
  using Type = T;
};

} // namespace base
