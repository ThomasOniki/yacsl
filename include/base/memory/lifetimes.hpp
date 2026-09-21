#pragma once

#include "base/attributes.hpp"
#include "base/meta/type_identity.hpp"
#include "base/ranges/slice.hpp"
#include "base/type_aliases.hpp"
#include "base/type_traits/qualifiers.hpp"
#include "base/type_traits/trivial.hpp"
#include "base/utils/is_constant_evaluated.hpp"
#include <string.h>

#ifndef BASE_NO_LIBCPP
#include <new> // IWYU pragma: keep
#endif

#ifdef BASE_NO_LIBCPP
[[nodiscard]] constexpr void* operator new(base::usize /*count*/,
                                           void* ptr) noexcept {
  return ptr;
}
#endif

namespace base::memory {

namespace _export_placement_new {
using ::operator new;
} // namespace _export_placement_new

template <typename T>
[[nodiscard]] constexpr type_traits::RemoveReference<T>&&
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
move(BASE_LIFETIMEBOUND T&& value) {
  return static_cast<type_traits::RemoveReference<T>&&>(value);
}

template <typename T>
[[nodiscard]] constexpr T&&
// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
forward(BASE_LIFETIMEBOUND type_traits::RemoveReference<T>&& value) {
  return static_cast<T&&>(value);
}
template <typename T>
[[nodiscard]] constexpr T&&
forward(BASE_LIFETIMEBOUND type_traits::RemoveReference<T>& value) {
  return static_cast<T&&>(value);
}

template <typename T, typename... Args>
constexpr T* construct_at(T* location BASE_LIFETIMEBOUND, Args&&... args) {
  return ::new (location) T(memory::forward<Args>(args)...);
}

template <type_traits::MutableType T>
constexpr ranges::Slice<T>
copy(ranges::Slice<meta::TypeIdentity<T> const> src,
     ranges::Slice<T> dst                       BASE_LIFETIMEBOUND) noexcept {
  usize const size = dst.size() < src.size() ? dst.size() : src.size();
  if (!utils::is_constant_evaluated() && type_traits::TriviallyCopyable<T>) {
    return {
        static_cast<T*>(::memcpy(dst.data(), src.data(), size * sizeof(T))),
        size,
    };
  }
  for (usize i = 0; i < size; ++i) {
    dst[i] = src[i];
  }
  return {dst.data(), size};
}
template <type_traits::MutableType T>
constexpr ranges::Slice<T> copy(ranges::Slice<meta::TypeIdentity<T> const> src,
                                T* dst BASE_LIFETIMEBOUND) noexcept {
  if (!utils::is_constant_evaluated() && type_traits::TriviallyCopyable<T>) {
    return {
        static_cast<T*>(::memcpy(dst, src.data(), src.size() * sizeof(T))),
        src.size(),
    };
  }
  for (usize i = 0; i < src.size(); ++i) {
    dst[i] = src[i];
  }
  return {dst, src.size()};
}

} // namespace base::memory
