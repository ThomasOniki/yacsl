#pragma once

#include "yacsl/attributes.hpp"
#include "yacsl/containers/stack_array.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/ranges/integer_range.hpp"
#include "yacsl/ranges/slice.hpp"
#include "yacsl/type_aliases.hpp"
#include "yacsl/type_traits/categories.hpp"
#include "yacsl/type_traits/qualifiers.hpp"

namespace yacsl::containers {

// THIS DOES NOT KEEP TRACK OF WHICH ELEMENTS WHERE CREATED AND DESTROYED, YOU
// ARE RESPONSIBLE FOR THAT.

template <typename T, usize N, type_traits::Integral SizeT = usize>
class StackArrayStorage {
public:
  template <typename... Args>
  constexpr void construct_at(this StackArrayStorage& self, SizeT index,
                              Args&&... args) {
    memory::construct_at(self.data() + index, memory::forward<Args>(args)...);
  }

  constexpr void destruct_at(this StackArrayStorage& self, SizeT index) {
    self.data()[index].~T();
  }

  constexpr void construct_all(this StackArrayStorage& self) {
    for (usize i = 0; i < N; ++i) {
      memory::construct_at(self.data() + i);
    }
  }
  constexpr void construct_all(this StackArrayStorage& self, T const& value) {
    for (usize i = 0; i < N; ++i) {
      memory::construct_at(self.data() + i, value);
    }
  }

  constexpr void destruct_all() {
    for (usize i = 0; i < N; ++i) {
      data()[i].~T();
    }
  }

  template <typename Self>
  [[nodiscard]] constexpr auto*
  data(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return reinterpret_cast<type_traits::CopyCV<Self, T>*>(
        memory::forward<Self>(self).data_.data());
  }

  [[nodiscard]] constexpr SizeT
  size(this StackArrayStorage const& /*self*/) noexcept {
    return static_cast<SizeT>(N);
  }

  template <typename Self>
  [[nodiscard]] constexpr auto&& operator[](this Self&& self,
                                            SizeT       index) noexcept {
    return memory::forward<Self>(self).data()[index];
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept
      -> ranges::Slice<type_traits::CopyCV<Self, T>, SizeT> {
    return {memory::forward<Self>(self).data(), static_cast<SizeT>(N)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  reversed_slice(this Self&& self YACSL_LIFETIMEBOUND) noexcept
      -> ranges::ReversedSlice<type_traits::CopyCV<Self, T>, SizeT> {
    return {memory::forward<Self>(self).data(), static_cast<SizeT>(N)};
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  bytes(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).slice().bytes();
  }

  [[nodiscard]] constexpr ranges::IntegerRange<SizeT>
  indices(this StackArrayStorage const& /*self*/) noexcept {
    return {0, N};
  }

  template <typename Self>
  [[nodiscard]] constexpr auto
  begin(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data();
  }
  template <typename Self>
  [[nodiscard]] constexpr auto
  end(this Self&& self YACSL_LIFETIMEBOUND) noexcept {
    return memory::forward<Self>(self).data() + N;
  }

private:
  alignas(T) StackArray<uchar, N * sizeof(T)> data_;
};

} // namespace yacsl::containers
