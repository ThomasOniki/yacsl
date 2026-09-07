#pragma once

#include "base/attributes.hpp"
#include "base/metaprogramming/type_identity.hpp"
#include "base/ranges/slice.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/type_traits/trivial.hpp"
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

template <typename T, typename... Args>
constexpr T* construct_at(T* location BASE_LIFETIMEBOUND, Args&&... args) {
  return ::new (location) T(base::forward<Args>(args)...);
}

template <MutableType T>
constexpr Slice<T> copy(Slice<TypeIdentity<T> const> src,
                        Slice<T> dst                 BASE_LIFETIMEBOUND) noexcept {
  usize const size = dst.size() < src.size() ? dst.size() : src.size();
  if !consteval {
    if (TriviallyCopyable<T>) {
      return {static_cast<T*>(::memcpy(dst.data(), src.data(), size)), size};
    }
  }
  for (usize i = 0; i < size; ++i) {
    dst[i] = src[i];
  }
  return {dst.data(), size};
}
template <MutableType T>
constexpr Slice<T> copy(Slice<TypeIdentity<T> const> src, T* dst BASE_LIFETIMEBOUND) noexcept {
  if !consteval {
    if (TriviallyCopyable<T>) {
      return {static_cast<T*>(::memcpy(dst, src.data(), src.size())), src.size()};
    }
  }
  for (usize i = 0; i < src.size(); ++i) {
    dst[i] = src[i];
  }
  return {dst, src.size()};
}

} // namespace base
