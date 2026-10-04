#pragma once

#include "yacsl/containers/stack_array.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/concepts.hpp"
#include "yacsl/meta/conditional.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/type_aliases.hpp"

namespace yacsl::math {

template <meta::Arithmetic T, usize Rows, usize Cols,
          meta::Integral SizeT = usize>
class Matrix {
public:
  template <typename Self>
  [[nodiscard]] constexpr meta::CopyCVRef<Self&&, T>
  operator[](this Self&& self, SizeT row, SizeT column) noexcept {
    if constexpr (Cols == 1) {
      return memory::forward<Self>(self).data_[row];
    } else {
      return memory::forward<Self>(self).data_[row][column];
    }
  }

  template <typename Self>
  [[nodiscard]] constexpr meta::CopyCVRef<Self&&, T>
  operator[](this Self&& self, SizeT column) noexcept
    requires(Cols == 1)
  {
    return memory::forward<Self>(self).data_[column];
  }

  template <usize N>
  [[nodiscard]] constexpr Matrix<T, Rows, N, SizeT>
  operator*(this Matrix&                     self,
            Matrix<T, Cols, N, SizeT> const& other) noexcept {
    Matrix<T, Rows, N, SizeT> result;
    for (usize i = 0; i < N; ++i) {
      for (usize j = 0; j < Rows; ++j) {
        for (usize k = 0; k < Cols; ++k) {
          result[j, i] += self[j, k] * other[k, i];
        }
      }
    }
    return result;
  }

  constexpr Matrix& operator*=(this Matrix& self, T factor) noexcept {
    for (usize row = 0; row < Rows; ++row) {
      for (usize col = 0; col < Cols; ++col) {
        self[row, col] *= factor;
      }
    }
    return self;
  }
  [[nodiscard]] constexpr Matrix operator*(this Matrix& self,
                                           T            factor) noexcept {
    Matrix result = self;
    result *= factor;
    return result;
  }

  template <typename Self> auto flat_slice(this Self&& self) noexcept {
    return memory::forward<Self>(self).data_.slice();
  }
  template <typename Self> auto flat_reversed_slice(this Self&& self) noexcept {
    return memory::forward<Self>(self).data_.reversed_slice();
  }

  template <typename Self> auto bytes(this Self&& self) noexcept {
    return memory::forward<Self>(self).data_.bytes();
  }

public:
  meta::Conditional<Cols == 1, containers::StackArray<T, Rows, SizeT>,
                    containers::StackArray<
                        containers::StackArray<T, Cols, SizeT>, Rows, SizeT>>
      data_;
};

template <meta::Arithmetic T, meta::Integral SizeT = usize>
using Vec2 = Matrix<T, 2, 1, SizeT>;
template <meta::Arithmetic T, meta::Integral SizeT = usize>
using Vec3 = Matrix<T, 3, 1, SizeT>;
template <meta::Arithmetic T, meta::Integral SizeT = usize>
using Vec4 = Matrix<T, 4, 1, SizeT>;

template <meta::Arithmetic T, meta::Integral SizeT = usize>
using Mat2x2 = Matrix<T, 2, 2, SizeT>;
template <meta::Arithmetic T, meta::Integral SizeT = usize>
using Mat3x3 = Matrix<T, 3, 3, SizeT>;
template <meta::Arithmetic T, meta::Integral SizeT = usize>
using Mat4x4 = Matrix<T, 4, 4, SizeT>;

} // namespace yacsl::math
