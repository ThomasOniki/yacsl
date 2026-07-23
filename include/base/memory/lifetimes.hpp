#pragma once

#include "base/attributes.hpp"
#include "base/ranges/slice.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/utilities/is_constant_evaluated.hpp"
#include "base/utilities/type_aliases.hpp"
#include <string.h>

#ifndef BASE_NO_LIBCPP
#include <new> // IWYU pragma: keep
#endif

#ifdef BASE_NO_LIBCPP
[[nodiscard]] constexpr void* operator new(base::usize /*count*/, void* ptr) noexcept {
  return ptr;
}
#endif

namespace base {

namespace _export_placement_new {
using ::operator new;
} // namespace _export_placement_new

template <typename T> // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
[[nodiscard]] constexpr RemoveReference<T>&& move(BASE_LIFETIMEBOUND T&& value) {
  return static_cast<RemoveReference<T>&&>(value);
}

template <typename T> // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
[[nodiscard]] constexpr T&& forward(BASE_LIFETIMEBOUND RemoveReference<T>&& value) {
  return static_cast<T&&>(value);
}
template <typename T>
[[nodiscard]] constexpr T&& forward(BASE_LIFETIMEBOUND RemoveReference<T>& value) {
  return static_cast<T&&>(value);
}

template <typename T, typename... Args> constexpr T* construct_at(T* location, Args&&... args) {
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  return ::new (static_cast<void*>(location)) T(base::forward<Args>(args)...);
}

template <typename T> constexpr Slice<T> copy(Slice<const T> src, Slice<T> dst) noexcept {
  const usize size = dst.size() < src.size() ? dst.size() : src.size();
  if (is_constant_evaluated() || !TriviallyCopyable<T>) {
    for (usize i = 0; i < size; ++i) {
      dst[i] = src[i];
    }
  } else {
    ::memcpy(dst.data(), src.data(), size);
  }
  return {dst.data(), size};
}
template <typename T> constexpr Slice<T> copy(Slice<const T> src, T* dst) noexcept {
  if (is_constant_evaluated() || !TriviallyCopyable<T>) {
    for (usize i = 0; i < src.size(); ++i) {
      dst[i] = src[i];
    }
  } else {
    ::memcpy(dst, src.data(), src.size());
  }
  return {dst, src.size()};
}

} // namespace base
